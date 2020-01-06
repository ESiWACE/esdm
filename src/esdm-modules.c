/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief ESDM module registry that keeps track of available backends.
 *
 */

#include <esdm-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: remove define on
#define ESDM_HAS_MD_POSIX
#ifdef ESDM_HAS_MD_POSIX
#  include "backends-metadata/posix/md-posix.h"
#  pragma message("Building ESDM with support generic 'MD-POSIX' backend.")
#endif


#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("MODULES", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("MODULES", fmt, __VA_ARGS__)

#define ESDM_HAS_MONDODB
#ifdef ESDM_HAS_MONGODB
#  include "backends-metadata/mongodb/mongodb.h"
#  pragma message("Building ESDM with MongoDB support.")
#endif

esdm_modules_t *esdm_modules_init(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  // Setup module registry
  esdm_modules_t *modules = NULL;
  esdm_backend_t *backend = NULL;

  modules = (esdm_modules_t *)malloc(sizeof(esdm_modules_t));

  esdm_config_backends_t *config_backends = esdm_config_get_backends(esdm);
  esdm_config_backend_t *b = NULL;

  esdm_config_backend_t *metadata_coordinator = esdm_config_get_metadata_coordinator(esdm);

  // Register metadata backend (singular)
  // TODO: This backend is meant as metadata coordinator in a hierarchy of MD (later)
  if (strncmp(metadata_coordinator->type, "metadummy", 9) == 0) {
    modules->metadata_backend = metadummy_backend_init(metadata_coordinator);
  }
#ifdef ESDM_HAS_MONGODB
  else if (strncmp(metadata_coordinator->type, "mongodb", 7) == 0) {
    modules->metadata_backend = mongodb_backend_init(metadata_coordinator);
  }
#endif
  else {
    ESDM_ERROR("Unknown metadata backend type. Please check your ESDM configuration.");
  }

  // Register data backends
  modules->data_backend_count = config_backends->count;
  modules->data_backends = malloc(config_backends->count * sizeof(esdm_backend_t *));

  for (int i = 0; i < config_backends->count; i++) {
    b = config_backends->backends[i];

    DEBUG("Backend config: %d, %s, %s, %s\n", i,
    b->type,
    b->id,
    b->target);

    backend = esdmI_init_backend(b->type, b);
    if(! backend){
      ESDM_ERROR_FMT("Unknown backend type: %s. Please check your ESDM configuration.", b->type);
    }
    backend->config = b;
    modules->data_backends[i] = backend;
  }
  free(config_backends->backends);  //esdmI_init_backend() took possession of the individual backend config objects in this array, so we only need to get rid of the array itself
  free(config_backends);

  esdm->modules = modules;
  return modules;
}

esdm_status esdm_modules_finalize(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  if (esdm->modules) {
    for(int i = esdm->modules->data_backend_count - 1 ; i >= 0; i--){
      esdm_backend_t *backend =esdm->modules->data_backends[i];
      if(backend->callbacks.finalize){
        backend->callbacks.finalize(backend);
      }
    }
    free(esdm->modules->data_backends);

    if(esdm->modules->metadata_backend->callbacks.finalize) {
      esdm->modules->metadata_backend->callbacks.finalize(esdm->modules->metadata_backend);
    }

    free(esdm->modules);
    esdm->modules = NULL;
  }

  return ESDM_SUCCESS;
}

esdm_backend_t** esdm_modules_makeBackendRecommendation(esdm_modules_t* modules, esdm_dataspace_t* space, int64_t* out_moduleCount, int64_t* out_maxFragmentSize) {
  eassert(out_moduleCount);

  //trivial implementation for now: just return a copy of our modules array
  *out_moduleCount = modules->data_backend_count;
  esdm_backend_t** result = ea_memdup(modules->data_backends, *out_moduleCount*sizeof(*result));

  if(out_maxFragmentSize) {
    //determine the minimal max_fragment_size of a data backend that we return
    *out_maxFragmentSize = INT64_MAX;
    for(int64_t i = 0; i < *out_moduleCount; i++) {
      if(*out_maxFragmentSize > result[i]->config->max_fragment_size) *out_maxFragmentSize = result[i]->config->max_fragment_size;
    }
  }

  return result;
}

esdm_status esdm_modules_get_by_type(esdm_module_type_t type, esdm_module_type_array_t **array) {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}
