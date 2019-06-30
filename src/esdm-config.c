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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>

#include <glib.h>

#include <esdm-internal.h>
#include <esdm.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("CONFIG", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("CONFIG", fmt, __VA_ARGS__)


esdm_config_t *esdm_config_init_from_str(const char *config_str) {
  esdm_config_t *config = NULL;
  config                = (esdm_config_t *)malloc(sizeof(esdm_config_t));
  config->json          = load_json(config_str); // parse text into JSON structure

  return config;
}


esdm_config_t *esdm_config_init(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  char *config_str = esdm_config_gather(0, NULL);

  esdm->config = esdm_config_init_from_str(config_str);
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
  read_file("_esdm.conf", &config_json);
  return config_json;
}


esdm_config_backend_t *esdm_config_get_metadata_coordinator(esdm_instance_t *esdm) {
  DEBUG_ENTER;

  json_t *root = (json_t *)esdm->config->json;

  json_t * esdm_e, * md_e, * type_e, *elem;
  esdm_e         = json_object_get(root, "esdm");
  md_e           = json_object_get(esdm_e, "metadata");
  type_e         = json_object_get(md_e, "type");
  DEBUG("json_path_get (metadata backend) => %p -> %s\n", type_e, json_string_value(type_e));

  esdm_config_backend_t *config_backend = (esdm_config_backend_t *)malloc(sizeof(esdm_config_backend_t));
  config_backend->type                  = json_string_value(type_e);
  config_backend->esdm                  = root;
  config_backend->backend               = md_e;

  elem                   = json_object_get(config_backend->backend, "id");
  config_backend->id     = json_string_value(elem);
  elem                   = json_object_get(config_backend->backend, "target");
  config_backend->target = json_string_value(elem);

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
  } else
    config_backend->data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;
  return config_backend;
}


esdm_config_backends_t *esdm_config_get_backends(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  json_t *root = (json_t *)esdm->config->json;

  json_t * esdm_e;
  esdm_e         = json_object_get(root, "esdm");

  // fetch configured backends
  json_t *element = json_object_get(esdm_e, "backends");

  esdm_config_backends_t *config_backends = (esdm_config_backends_t *)malloc(sizeof(esdm_config_backends_t));

  if (element) {
    if (json_typeof(element) == JSON_ARRAY) {
      // Element is array, therefor may contain valid backend configurations
      size_t size = json_array_size(element);

      esdm_config_backend_t *backends;
      backends = (esdm_config_backend_t *)malloc(sizeof(esdm_config_backend_t) * size);

      //printf("JSON Array of %ld element%s:\n", size, json_plural(size));

      size_t i, j;
      for (i = 0; i < size; i++) {
        //print_json_aux(json_array_get(element, i), 0);

        json_t *backend = json_array_get(element, i);
        json_t *elem    = NULL;

        elem             = json_object_get(backend, "type");
        backends[i].type = json_string_value(elem);

        elem           = json_object_get(backend, "id");
        backends[i].id = json_string_value(elem);
        for (j = 0; j < i; j++) {
          if (strcmp(backends[i].id, backends[j].id) == 0) {
            printf("ERROR two backends with the same ID found: %s\n", backends[i].id);
            ESDM_ERROR("Aborting!");
          }
        }

        elem                          = json_object_get(backend, "target");
        backends[i].target            = json_string_value(elem);
        backends[i].performance_model = json_object_get(backend, "performance-model");
        DEBUG("type=%s id = %s target=%s\n", backends[i].type,
        backends[i].id,
        backends[i].target);

        elem = json_object_get(backend, "max-threads-per-node");
        if (elem == NULL) {
          backends[i].max_threads_per_node = 0;
        } else {
          backends[i].max_threads_per_node = json_integer_value(elem);
        }

        elem = json_object_get(backend, "max-global-threads");
        if (elem == NULL) {
          backends[i].max_global_threads = 0;
        } else {
          backends[i].max_global_threads = json_integer_value(elem);
        }

        elem = json_object_get(backend, "accessibility");
        if (elem != NULL) {
          const char *str = json_string_value(elem);
          if (strcasecmp(str, "global") == 0) {
            backends[i].data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;
          } else if (strcasecmp(str, "local") == 0) {
            backends[i].data_accessibility = ESDM_ACCESSIBILITY_NODELOCAL;
          } else {
            ESDM_ERROR("Unknown accessibility!");
          }
        } else
          backends[i].data_accessibility = ESDM_ACCESSIBILITY_GLOBAL;

        elem = json_object_get(backend, "max-fragment-size");
        if (elem == NULL) {
          backends[i].max_fragment_size = 10 * 1024 * 1024;
        } else {
          backends[i].max_fragment_size = json_integer_value(elem);
        }

        backends[i].esdm    = root;
        backends[i].backend = backend;
      }

      config_backends->count    = size;
      config_backends->backends = backends;
    }
  } else {
    ESDM_ERROR("Invalid configuration! /esdm/backends is not an array.");
  }

  return config_backends;
}
