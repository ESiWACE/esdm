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
 * @brief The site configuration describes the data center or subcomponents.
 *
 */

#include <esdm-internal.h>
#include <esdm.h>
#include <fcntl.h>
#include <glib.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("CONFIG", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("CONFIG", fmt, __VA_ARGS__)

esdm_config_t *esdm_config_init_from_str(const char *config_str) {
  json_t* json = load_json(config_str); // parse text into JSON structure
  if(! json) {
    ESDM_ERROR_FMT("CONFIG invalid JSON config:\"%s\"", config_str);
  }

  esdm_config_t *config;
  config = ea_checked_malloc(sizeof(esdm_config_t));
  config->json = json;

  json_t* esdm_e = json_object_get(json, "esdm");
  if(! esdm_e) ESDM_ERROR("Configuration: esdm tag not set");

  config->boundListImplementation = BOUND_LIST_IMPLEMENTATION_BTREE;  //default
  json_t* boundListImplementation_e = json_object_get(esdm_e, "bound list implementation");
  if(boundListImplementation_e) {
    const char* selection = json_string_value(boundListImplementation_e);
    if(!selection) {
      ESDM_ERROR("Configuration: \"bound list implementation\" tag is not a string");
    } else if(!strcmp(selection, "array")) {
      config->boundListImplementation = BOUND_LIST_IMPLEMENTATION_ARRAY;
    } else if(!strcmp(selection, "btree")) {
      config->boundListImplementation = BOUND_LIST_IMPLEMENTATION_BTREE;
    } else {
      ESDM_ERROR_FMT("Configuration: unrecognized value of \"bound list implementation\" tag: \"%s\"", selection);
    }
  }

  return config;
}

esdm_config_t *esdm_config_init(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  char *config_str = esdm_config_gather(0, NULL);
  if(config_str == NULL){
    return NULL;
  }
  esdm->config = esdm_config_init_from_str(config_str);
  free(config_str);
  return esdm->config;
}

esdm_status esdm_config_finalize(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  if (esdm->config) {
    json_decref(esdm->config->json);
    free(esdm->config);
    esdm->config = NULL;
  }

  return ESDM_SUCCESS;
}

/**
 * TODO:
 *	/etc/esdm/esdm.conf
 *	~/.config/esdm/esdm.conf
 *	~/.esdm.conf
 *  ./esdm.conf
 *  environment variable
 *  arguments
 *
 */
char *esdm_config_gather() {
  ESDM_DEBUG(__func__);

  char *config_json = NULL;
  ea_read_file("esdm.conf", & config_json);
  return config_json;
}

esdm_config_backend_t *esdm_config_get_metadata_coordinator(esdm_instance_t *esdm) {
  DEBUG_ENTER;

  json_t *root = (json_t *)esdm->config->json;

  json_t *esdm_e, *md_e, *type_e, *elem;
  esdm_e = json_object_get(root, "esdm");
  if(! esdm_e){
    ESDM_ERROR("Configuration: esdm tag not set");
  }


  md_e = json_object_get(esdm_e, "metadata");
  if(! md_e){
    ESDM_ERROR("Configuration: metadata not set");
  }

  type_e = json_object_get(md_e, "type");
  if(! type_e){
    ESDM_ERROR("Configuration: type not set");
  }

  esdm_config_backend_t *config_backend = ea_checked_malloc(sizeof(esdm_config_backend_t));
  eassert(config_backend);
  config_backend->type = json_string_value(type_e); //FIXME (systematic error): Using the strings from jansson without copying forces us to keep the jansson data around after initialization. Copy the strings, interpret the data, and purge the jansson data from memory.
  config_backend->esdm = root;
  config_backend->backend = md_e;
  if(! md_e){
    ESDM_ERROR("Configuration: metadata object not loaded");
  }


  elem = json_object_get(config_backend->backend, "id");
  if(! elem){
    ESDM_ERROR("Configuration: ID not set");
  }
  config_backend->id = json_string_value(elem);
  eassert(config_backend->id != NULL);
  elem = json_object_get(config_backend->backend, "target");
  if(! elem){
    ESDM_ERROR("Configuration: target not set");
  }
  config_backend->target = json_string_value(elem);
  eassert(config_backend->target != NULL);

  elem = json_object_get(config_backend->backend, "accessibility");
  if (elem != NULL) {
    const char *str = json_string_value(elem);
    if (strcasecmp(str, "global") == 0) {
      config_backend->data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;
    } else if (strcasecmp(str, "local") == 0) {
      config_backend->data_accessibility = ESDM_ACCESSIBILITY_NODELOCAL;
    } else {
      ESDM_ERROR("Unknown accessibility!");
    }
  } else {
    config_backend->data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;
  }
  return config_backend;
}

esdm_config_backends_t *esdm_config_get_backends(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  json_t *root = (json_t *)esdm->config->json;

  json_t *esdm_e;
  esdm_e = json_object_get(root, "esdm");
  if(! esdm_e){
    ESDM_ERROR("Configuration: esdm tag not set");
  }

  // fetch configured backends
  json_t *elem = json_object_get(esdm_e, "backends");
  if(! elem){
    ESDM_ERROR("Configuration: backends not set");
  }
  esdm_config_backends_t *config_backends = ea_checked_malloc(sizeof(esdm_config_backends_t));

  if (elem) {
    if (json_typeof(elem) == JSON_ARRAY) {
      // Element is array, therefor may contain valid backend configurations
      size_t size = json_array_size(elem);

      esdm_config_backend_t **backends = ea_checked_malloc(size*sizeof(*backends));

      //printf("JSON Array of %ld elem%s:\n", size, json_plural(size));

      size_t i, j;
      for (i = 0; i < size; i++) {
        //print_json_aux(json_array_get(elem, i), 0);

        backends[i] = ea_checked_malloc(sizeof(*backends[i]));

        json_t *backend = json_array_get(elem, i);
        json_t *elem = NULL;

        elem = json_object_get(backend, "type");
        if(! elem){
          ESDM_ERROR("Configuration: type not set");
        }
        backends[i]->type = json_string_value(elem);

        elem = json_object_get(backend, "id");
        if(! elem){
          ESDM_ERROR("Configuration: id not set");
        }
        backends[i]->id = json_string_value(elem);
        for (j = 0; j < i; j++) {
          if (strcmp(backends[i]->id, backends[j]->id) == 0) {
            printf("ERROR two backends with the same ID found: %s\n", backends[i]->id);
            ESDM_ERROR("Aborting!");
          }
        }

        elem = json_object_get(backend, "target");
        if(! elem){
          ESDM_ERROR("Configuration: target not set");
        }
        backends[i]->target = json_string_value(elem);
        backends[i]->performance_model = json_object_get(backend, "performance-model");
        DEBUG("type=%s id = %s target=%s\n", backends[i]->type,
        backends[i]->id,
        backends[i]->target);

        elem = json_object_get(backend, "max-threads-per-node");
        if (elem == NULL) {
          backends[i]->max_threads_per_node = 0;
        } else {
          backends[i]->max_threads_per_node = json_integer_value(elem);
        }

        elem = json_object_get(backend, "write-stream-blocksize");
        if (elem == NULL) {
          backends[i]->write_stream_blocksize = 0;
        } else {
          backends[i]->write_stream_blocksize = json_integer_value(elem);
        }

        elem = json_object_get(backend, "max-global-threads");
        if (elem == NULL) {
          backends[i]->max_global_threads = 0;
        } else {
          backends[i]->max_global_threads = json_integer_value(elem);
        }

        elem = json_object_get(backend, "accessibility");
        if (elem != NULL) {
          const char *str = json_string_value(elem);
          if (strcasecmp(str, "global") == 0) {
            backends[i]->data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;
          } else if (strcasecmp(str, "local") == 0) {
            backends[i]->data_accessibility = ESDM_ACCESSIBILITY_NODELOCAL;
          } else {
            ESDM_ERROR("Unknown accessibility!");
          }
        } else
          backends[i]->data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;

        elem = json_object_get(backend, "max-fragment-size");
        if (elem == NULL) {
          backends[i]->max_fragment_size = 10 * 1024 * 1024;
        } else {
          backends[i]->max_fragment_size = json_integer_value(elem);
        }

        elem = json_object_get(backend, "fragmentation-method");
        backends[i]->fragmentation_method = ESDMI_FRAGMENTATION_METHOD_CONTIGUOUS; //set the default
        if(elem && json_typeof(elem) == JSON_STRING) {
          if(!strcmp(json_string_value(elem), "contiguous")) {
            backends[i]->fragmentation_method = ESDMI_FRAGMENTATION_METHOD_CONTIGUOUS;
          } else if(!strcmp(json_string_value(elem), "equalized")) {
            backends[i]->fragmentation_method = ESDMI_FRAGMENTATION_METHOD_EQUALIZED;
          } else {
            ESDM_ERROR("Unrecognized value of \"fragmentation-method\"");
          }
        }

        backends[i]->esdm = root;
        backends[i]->backend = backend;
      }

      config_backends->count = size;
      config_backends->backends = backends;
    }
  } else {
    ESDM_ERROR("Invalid configuration! /esdm/backends is not an array.");
  }

  return config_backends;
}
