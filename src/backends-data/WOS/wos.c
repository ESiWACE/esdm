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
 *
 */

/**
 * @file
 * @brief A data backend to provide wos compatibility.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <esdm-datatypes.h>
#include <esdm-debug.h>
#include <esdm.h>

#include "wos.h"

#define PAGE_4K (4096ULL)
#define BLOCKSIZE (PAGE_4K)
#define BLOCKMASK (BLOCKSIZE - 1)
#define WOS_HOST "host"
#define WOS_POLICY "policy"
#define WOS_OBJ_NUM 1
#define WOS_OBJECT_ID "object_id"
#define DEBUG(fmt) ESDM_DEBUG(fmt)
#define DEBUG_FMT(fmt, ...) ESDM_DEBUG_COM_FMT("WOS", fmt, __VA_ARGS__)

int wos_get_param(const char *conf, char **output, const char *param) {
  if (!conf || !output) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }
  *output = NULL;

  char *key = strstr(conf, param);
  if (!key) {
    DEBUG("Parameter not found");
    return ESDM_ERROR;
  }
  char *value = strchr(key, '=');
  if (!value || !*value) {
    DEBUG("Parameter not found");
    return ESDM_ERROR;
  }
  value++;
  char *end_value = strchr(value, ';');
  if (!end_value) {
    DEBUG("Parameter not found");
    return ESDM_ERROR;
  }
  size_t size = end_value - value;
  char _output[1 + size];
  strncpy(_output, value, size);
  _output[size] = 0;
  *output = strdup(_output);
  if (!*output) {
    DEBUG("Memory error");
    return ESDM_ERROR;
  }

  return ESDM_SUCCESS;
}

int wos_get_host(const char *conf, char **host) {
  return wos_get_param(conf, host, WOS_HOST);
}

int wos_get_policy(const char *conf, char **policy) {
  return wos_get_param(conf, policy, WOS_POLICY);
}

void wos_delete_oid_list(esdm_backend_t_wos_t *ebm) {
  if (ebm->oid_list) {
    int j = 0;
    while (ebm->oid_list[j])
      DeleteWosWoid(ebm->oid_list[j++]);
    free(ebm->oid_list);
    ebm->oid_list = NULL;
  }
  if (ebm->size_list) {
    free(ebm->size_list);
    ebm->size_list = NULL;
  }
}

int wos_object_list_encode(t_WosOID **oid_list, char **out_object_id) {
  if (!out_object_id) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }
  *out_object_id = NULL;

  if (oid_list) {
    int i;
    const char *current;
    for (i = 0; oid_list[i]; ++i) {
      current = OIDtoString(oid_list[i]);
      if (*out_object_id) {
        char buffer[strlen(*out_object_id) + strlen(current) + 2];
        sprintf(buffer, "%s,%s", *out_object_id, current);
        free(*out_object_id);
        *out_object_id = strdup(buffer);
      } else
        *out_object_id = strdup(current);
      if (!out_object_id) {
        DEBUG("Memory error");
        return ESDM_ERROR;
      }
    }
  }

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_init(const char *conf, esdm_backend_t *eb) {
  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  ebm->wos_cluster = NULL;
  ebm->wos_policy = NULL;
  ebm->wos_meta_obj = NULL;
  ebm->oid_list = NULL;
  ebm->size_list = NULL;

  char *host = NULL;
  if (wos_get_host(conf, &host) || !host) {
    DEBUG("Unable to get host address");
    return ESDM_ERROR;
  }
  ebm->wos_cluster = Conn(host);
  if (!ebm->wos_cluster) {
    free(host);
    DEBUG("Unable to connect to cluster");
    return ESDM_ERROR;
  }
  free(host);

  char *policy = NULL;
  if (wos_get_policy(conf, &policy) || !policy) {
    DeleteWosCluster(ebm->wos_cluster);
    ebm->wos_cluster = NULL;
    DEBUG("Unable to get policy name");
    return ESDM_ERROR;
  }
  ebm->wos_policy = GetPolicy(ebm->wos_cluster, policy);
  if (!ebm->wos_policy) {
    DeleteWosCluster(ebm->wos_cluster);
    ebm->wos_cluster = NULL;
    free(policy);
    DEBUG("Unable to get policy");
    return ESDM_ERROR;
  }
  free(policy);

  // Performance model
  esdm_backend_t_init_dynamic_perf_model_lat_thp(&ebm->perf_model);

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_fini(esdm_backend_t *eb) {
  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  int i;
  if (ebm->oid_list) {
    for (i = 0; ebm->oid_list[i]; i++) {
      DeleteWosWoid(ebm->oid_list[i]);
    }
    free(ebm->oid_list);
    ebm->oid_list = NULL;
  }
  if (ebm->size_list) {
    free(ebm->size_list);
    ebm->size_list = NULL;
  }

  if (ebm->wos_cluster) {
    DeleteWosCluster(ebm->wos_cluster);
    ebm->wos_cluster = NULL;
  }
  if (ebm->wos_policy) {
    DeleteWosPolicy(ebm->wos_policy);
    ebm->wos_policy = NULL;
  }

  // Performance model
  esdm_backend_t_finalize_dynamic_perf_model_lat_thp(&ebm->perf_model);

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_alloc(esdm_backend_t *eb, int n_dims, int *dims_size, esdm_type_t type, char **out_object_id, char **out_wos_metadata) {
  if (!out_object_id || !out_wos_metadata) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }
  *out_object_id = NULL;
  *out_wos_metadata = NULL;

  //int item_size = esdm_sizeof(type);
  int item_size = 1; // type is not used, input size is measured in bytes
  if (!dims_size || (n_dims < 1) || !item_size) {
    DEBUG("Wrong parameters");
    return ESDM_ERROR;
  }

  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  if (!ebm->wos_cluster || !ebm->wos_policy) {
    DEBUG("Unable to get wos parameters");
    return ESDM_ERROR;
  }

  t_WosStatus *wos_status = CreateStatus();
  if (!wos_status) {
    DEBUG("Unable to create wos status");
    return ESDM_ERROR;
  }

  int i;
  uint64_t num_value = 1;
  for (i = 0; i < n_dims; i++)
    num_value *= dims_size[i];
  uint64_t size = num_value * item_size;

  /* Set the best fragmentation */
  //obj_num = wos_get_fragmentation(num_value, size);     /* To be completed */
  int obj_num = WOS_OBJ_NUM;
  uint64_t obj_size = size / obj_num;
  uint64_t obj_offset = size % obj_num;

  /* Create object for internal use */
  //rc = wos_metaobj_create(&ebm->wos_meta_obj, md1, md2);        /* To be completed */

  /* Reserve oids for storing data objects - null terminated array */
  ebm->oid_list = ea_checked_calloc(obj_num + 1, sizeof(t_WosOID *));
  ebm->size_list = ea_checked_calloc(obj_num + 1, sizeof(uint64_t));

  for (i = 0; i < obj_num; i++) {
    ebm->oid_list[i] = CreateWoid();
    if (!ebm->oid_list[i]) {
      wos_delete_oid_list(ebm);
      DeleteWosStatus(wos_status);
      DEBUG("Unable to create the objects");
      return ESDM_ERROR;
    }
    ebm->size_list[i] = obj_offset && (i == (obj_num - 1)) ? obj_offset : obj_size;
    Reserve_b(wos_status, ebm->oid_list[i], ebm->wos_policy, ebm->wos_cluster);
    if (GetStatus(wos_status)) {
      wos_delete_oid_list(ebm);
      DeleteWosStatus(wos_status);
      DEBUG("Unable to create the objects");
      return ESDM_ERROR;
    }
  }

  if (wos_object_list_encode(ebm->oid_list, out_object_id)) {
    wos_delete_oid_list(ebm);
    DeleteWosStatus(wos_status);
    DEBUG("Unable to set output data");
    return ESDM_ERROR;
  }
  //wos_object_meta_encode(ebm, out_wos_metadata);  /* To be completed */

  DeleteWosStatus(wos_status);

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_open(esdm_backend_t *eb, char *object_id, void **obj_handle) {
  if (!object_id || !obj_handle) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }
  *obj_handle = NULL;

  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }
  if (!ebm->wos_cluster || !ebm->wos_policy) {
    DEBUG("Unable to get wos parameters");
    return ESDM_ERROR;
  }

  t_WosStatus *wos_status = CreateStatus();
  if (!wos_status) {
    DEBUG("Unable to create wos status");
    return ESDM_ERROR;
  }

  t_WosOID *oid = CreateWoid();
  if (!oid) {
    DeleteWosStatus(wos_status);
    DEBUG("Unable to create a wos oid");
    return ESDM_ERROR;
  }
  SetOID(oid, object_id);

  /*
	Exists_b(wos_status, oid, ebm->wos_cluster);
	if (GetStatus(wos_status)) {
		DeleteWosWoid(oid);
		DeleteWosStatus(wos_status);
		DEBUG("Unable to find the object");
		return ESDM_ERROR;
	}
*/

  *obj_handle = (void *)oid;

  DeleteWosStatus(wos_status);

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_delete(esdm_backend_t *eb, void *obj_handle) {
  if (!obj_handle) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }

  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }
  if (!ebm->wos_cluster || !ebm->wos_policy) {
    DEBUG("Unable to get wos parameters");
    return ESDM_ERROR;
  }

  t_WosStatus *wos_status = CreateStatus();
  if (!wos_status) {
    DEBUG("Unable to create wos status");
    return ESDM_ERROR;
  }

  Delete_b(wos_status, (t_WosOID *)obj_handle, ebm->wos_cluster);
  if (GetStatus(wos_status)) {
    DeleteWosStatus(wos_status);
    DEBUG("Unable to allocate data");
    return ESDM_ERROR;
  }

  DeleteWosStatus(wos_status);

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_write(esdm_backend_t *eb, void *obj_handle, uint64_t start, uint64_t count, esdm_type_t type, void *data) {
  if (!obj_handle) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }
  if (!data || !count)
    return esdm_backend_t_wos_delete(eb, obj_handle);

  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }
  if (!ebm->wos_cluster || !ebm->wos_policy) {
    DEBUG("Unable to get wos parameters");
    return ESDM_ERROR;
  }
  if (!ebm->size_list || !ebm->size_list[0]) {
    DEBUG("Unable to get object size");
    return ESDM_ERROR;
  }
  uint64_t obj_size = ebm->size_list[0];

  //int item_size = esdm_sizeof(type);
  int item_size = 1; // type is not used, input size is measured in bytes

  // TODO: start and count has to be used to write the appropriate objects; only one object is now created
  start *= item_size;
  count *= item_size;
  if (obj_size < start + count) {
    DEBUG("Wrong parameters start or count");
    return ESDM_ERROR;
  }

  void *buffer = ea_checked_calloc(obj_size, sizeof(char));
  if (!buffer) {
    DEBUG("Memory error");
    return ESDM_ERROR;
  }

  t_WosStatus *wos_status = CreateStatus();
  if (!wos_status) {
    free(buffer);
    DEBUG("Unable to create wos status");
    return ESDM_ERROR;
  }

  int i, rc = 0;
  t_WosObjPtr *wobj = NULL;
  for (i = 0; ebm->oid_list[i]; i++) {
    wobj = WosObjCreate();
    if (!wobj) {
      rc = -1;
      DEBUG("Unable to create wos object");
      break;
    }
    DEBUG("New object created");

    /* Fill data */
    // TODO: start and count has to be used to write the appropriate objects; only one object is now created
    bzero(buffer, ebm->size_list[i]);
    memcpy(buffer, (char *)data + start, count);
    DEBUG("Data copied in local buffer");

    SetDataObj(buffer, ebm->size_list[i], wobj);
    DEBUG("Data prepared to be sent");

    PutOID_b(wos_status, ebm->oid_list[i], wobj, ebm->wos_cluster);
    if ((rc = GetStatus(wos_status))) {
      DeleteWosObj(wobj);
      DEBUG("Unable to allocate data");
      break;
    }
    DEBUG_FMT("Data sent to remote buffer (%d bytes)\n", count);

    DeleteWosObj(wobj);
    DEBUG("Hook removed");
  }

  free(buffer);
  DeleteWosStatus(wos_status);

  return rc;
}

int esdm_backend_t_wos_read(esdm_backend_t *eb, void *obj_handle, uint64_t start, uint64_t count, esdm_type_t type, void *data) {
  if (!obj_handle) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }
  if (!data || !count) {
    DEBUG("No data has to be read");
    return ESDM_ERROR;
  }

  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }
  if (!ebm->wos_cluster || !ebm->wos_policy) {
    DEBUG("Unable to get wos parameters");
    return ESDM_ERROR;
  }

  t_WosStatus *wos_status = CreateStatus();
  if (!wos_status) {
    DEBUG("Unable to create wos status");
    return ESDM_ERROR;
  }

  t_WosObjPtr *wobj = WosObjHookCreate();
  if (!wobj) {
    DeleteWosStatus(wos_status);
    DEBUG("Unable to create wos object");
    return ESDM_ERROR;
  }
  DEBUG("New hook created");

  Get_b(wos_status, (t_WosOID *)obj_handle, wobj, ebm->wos_cluster);
  if (GetStatus(wos_status)) {
    DeleteWosObj(wobj);
    DEBUG("Hook removed");
    DeleteWosStatus(wos_status);
    DEBUG("Unable to allocate data");
    return ESDM_ERROR;
  }

  DeleteWosStatus(wos_status);

  const void *buffer = NULL;
  int len;
  GetDataObj(&buffer, &len, wobj);
  if (!buffer || !len) {
    DeleteWosObj(wobj);
    DEBUG("Hook removed");
    DEBUG("Unable to get data");
    return ESDM_ERROR;
  }
  //int item_size = esdm_sizeof(type);
  int item_size = 1; // type is not used, input size is measured in bytes

  // TODO: start and count has to be used to select the appropriate objects, get data and aggregate the result to be outputed; now there is only one object
  start *= item_size;
  count *= item_size;
  if ((uint64_t)len < start + count) {
    DeleteWosObj(wobj);
    DEBUG("Hook removed");
    DEBUG("Wrong parameters start or count");
    return ESDM_ERROR;
  }

  memcpy(data, (char *)buffer + start, count);
  DEBUG_FMT("Data received from remote buffer (%d bytes)\n", count);

  DeleteWosObj(wobj);
  DEBUG("Hook removed");

  return ESDM_SUCCESS;
}

int esdm_backend_t_wos_close(esdm_backend_t *eb, void *obj_handle) {
  if (!eb || !obj_handle) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }

  DeleteWosWoid((t_WosOID *)obj_handle);

  return ESDM_SUCCESS;
}

int wos_backend_performance_check(esdm_backend_t *eb, int data_size, float *out_time) {
  if (!eb || (data_size <= 0))
    return ESDM_ERROR;

  if (out_time)
    *out_time = 0.0;

  struct timeval t0, t1;

  char *object_id = NULL;
  char *object_meta = NULL;
  void *object_handle = NULL;

  char *data_w = ea_checked_malloc(data_size * sizeof(char));
  char *data_r = ea_checked_malloc(data_size * sizeof(char));

  if (!data_w || !data_r)
    return ESDM_ERROR;

  data_w[data_size - 1] = 0;
  data_r[data_size - 1] = 0;

  if (esdm_backend_t_wos_alloc(eb, 1, &data_size, SMD_DTYPE_CHAR, &object_id, &object_meta))
    goto fini;

  if (esdm_backend_t_wos_open(eb, object_id, &object_handle))
    goto fini;

  gettimeofday(&t0, NULL);
  if (esdm_backend_t_wos_write(eb, object_handle, 0, data_size, SMD_DTYPE_CHAR, data_w))
    goto close;
  gettimeofday(&t1, NULL);
  if (out_time)
    *out_time = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;

  if (esdm_backend_t_wos_read(eb, object_handle, 0, data_size, SMD_DTYPE_CHAR, data_r))
    goto close;

  if (esdm_backend_t_wos_write(eb, object_handle, 0, 0, SMD_DTYPE_CHAR, NULL))
    goto close;

close:
  if (esdm_backend_t_wos_close(eb, object_handle))
    goto fini;

fini:
  if (object_id)
    free(object_id);
  if (object_meta)
    free(object_meta);
  free(data_w);
  free(data_r);

  return ESDM_SUCCESS;
}

int wos_backend_performance_estimate(esdm_backend_t *eb, esdm_fragment_t *fragment, float *out_time) {
  if (!fragment || !out_time) {
    DEBUG("Null pointer");
    return ESDM_ERROR;
  }

  if (!eb) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  esdm_backend_t_wos_t *ebm = (esdm_backend_t_wos_t *)eb->data;
  if (!ebm) {
    DEBUG("Unable to get struct");
    return ESDM_ERROR;
  }

  return esdm_backend_t_estimate_dynamic_perf_model_lat_thp(&ebm->perf_model, fragment, out_time);
}

int esdm_backend_t_wos_fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *fragment, json_t *metadata) {
  char *obj_id = NULL;
  void *obj_handle = NULL;
  int rc = ESDM_SUCCESS;

  if (!backend || !fragment || !fragment->buf || !metadata)
    return ESDM_ERROR;

  const char *key;
  json_t *value;
  json_object_foreach(metadata, key, value) {
    if (!strcmp(key, WOS_OBJECT_ID)) {
      obj_id = strdup(json_string_value(value));
      break;
    }
  }
  while (!obj_id && fragment->metadata->size) {
    char *start_id = strstr(fragment->metadata->json, WOS_OBJECT_ID), *end_id;
    if (!start_id)
      break;
    start_id = strstr(start_id, ":");
    if (!start_id)
      break;
    start_id = strstr(start_id, "\"");
    if (!start_id)
      break;
    start_id++;
    end_id = strstr(start_id, "\"");
    if (!end_id)
      break;
    obj_id = strndup(start_id, end_id - start_id);
    break;
  }
  if (!obj_id) {
    DEBUG("Object not found");
    rc = ESDM_ERROR;
    goto _RETRIEVE_EXIT;
  }
  DEBUG(obj_id);

  rc = esdm_backend_t_wos_open(backend, obj_id, &obj_handle);
  if (rc) {
    goto _RETRIEVE_EXIT;
  }

  rc = esdm_backend_t_wos_read(backend, obj_handle, 0, fragment->bytes, fragment->dataspace->type, fragment->buf);
  if (rc) {
    esdm_backend_t_wos_close(backend, obj_handle);
    goto _RETRIEVE_EXIT;
  }

  rc = esdm_backend_t_wos_close(backend, obj_handle);
  if (rc) {
    goto _RETRIEVE_EXIT;
  }

_RETRIEVE_EXIT:

  return rc;
}

int esdm_backend_t_wos_fragment_update(esdm_backend_t *backend, esdm_fragment_t *fragment) {
  char *obj_id = NULL;
  char *obj_meta = NULL;
  void *obj_handle = NULL;
  int rc = ESDM_SUCCESS;

  if (!backend || !fragment)
    return ESDM_ERROR;

  while (fragment->metadata->size) {
    char *start_id = strstr(fragment->metadata->json, WOS_OBJECT_ID), *end_id;
    if (!start_id)
      break;
    start_id = strstr(start_id, ":");
    if (!start_id)
      break;
    start_id = strstr(start_id, "\"");
    if (!start_id)
      break;
    start_id++;
    end_id = strstr(start_id, "\"");
    if (!end_id)
      break;
    obj_id = strndup(start_id, end_id - start_id);
    break;
  }

  if (obj_id) {
    DEBUG("Object found: it needs to be replaced");
    rc = esdm_backend_t_wos_open(backend, obj_id, &obj_handle);
    if (rc) {
      goto _UPDATE_EXIT;
    }

    rc = esdm_backend_t_wos_delete(backend, obj_handle);
    if (rc) {
      esdm_backend_t_wos_close(backend, obj_handle);
      goto _UPDATE_EXIT;
    }

    rc = esdm_backend_t_wos_close(backend, obj_handle);
    if (rc) {
      goto _UPDATE_EXIT;
    }

    free(obj_id);
  }

  int size = fragment->bytes;
  rc = esdm_backend_t_wos_alloc(backend, 1, &size, fragment->dataspace->type, &obj_id, &obj_meta);
  if (rc) {
    goto _UPDATE_EXIT;
  }

  fragment->metadata->size += sprintf(&fragment->metadata->json[fragment->metadata->size], "{\"%s\" : \"%s\"}", WOS_OBJECT_ID, obj_id);
  if (fragment->metadata->size >= ESDM_MAX_SIZE) {
    rc = -ENOMEM;
    goto _UPDATE_EXIT;
  }

  rc = esdm_backend_t_wos_open(backend, obj_id, &obj_handle);
  if (rc) {
    goto _UPDATE_EXIT;
  }

  rc = esdm_backend_t_wos_write(backend, obj_handle, 0, fragment->bytes, fragment->dataspace->type, fragment->buf);
  if (rc) {
    esdm_backend_t_wos_close(backend, obj_handle);
    goto _UPDATE_EXIT;
  }

  rc = esdm_backend_t_wos_close(backend, obj_handle);
  if (rc) {
    goto _UPDATE_EXIT;
  }

_UPDATE_EXIT:
  free(obj_id);
  free(obj_meta);

  return rc;
}

int esdm_backend_t_wos_fragment_delete(esdm_backend_t *backend, esdm_fragment_t *fragment, json_t *metadata) {
  char *obj_id = NULL;
  void *obj_handle = NULL;
  int rc = ESDM_SUCCESS;

  if (!backend || !fragment)
    return ESDM_ERROR;

  const char *key;
  json_t *value;
  json_object_foreach(metadata, key, value) {
    if (!strcmp(key, WOS_OBJECT_ID)) {
      obj_id = strdup(json_string_value(value));
      break;
    }
  }
  while (!obj_id && fragment->metadata->size) {
    char *start_id = strstr(fragment->metadata->json, WOS_OBJECT_ID), *end_id;
    if (!start_id)
      break;
    start_id = strstr(start_id, ":");
    if (!start_id)
      break;
    start_id = strstr(start_id, "\"");
    if (!start_id)
      break;
    start_id++;
    end_id = strstr(start_id, "\"");
    if (!end_id)
      break;
    obj_id = strndup(start_id, end_id - start_id);
    break;
  }
  if (!obj_id) {
    DEBUG("Object not found");
    rc = -1;
    goto _DELETE_EXIT;
  }

  rc = esdm_backend_t_wos_open(backend, obj_id, &obj_handle);
  if (rc) {
    goto _DELETE_EXIT;
  }

  rc = esdm_backend_t_wos_delete(backend, obj_handle);
  if (rc) {
    esdm_backend_t_wos_close(backend, obj_handle);
    goto _DELETE_EXIT;
  }

  rc = esdm_backend_t_wos_close(backend, obj_handle);
  if (rc) {
    goto _DELETE_EXIT;
  }

_DELETE_EXIT:

  return rc;
}

int esdm_backend_t_wos_fragment_mkfs(esdm_backend_t *backend, int enforce_format) {
  if (!backend)
    return ESDM_ERROR;

  return ESDM_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
  ///////////////////////////////////////////////////////////////////////////////
  // NOTE: This serves as a template for the posix plugin and is memcopied!    //
  ///////////////////////////////////////////////////////////////////////////////
  .name = "WOS",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    (int (*)())esdm_backend_t_wos_fini, // finalize
    wos_backend_performance_estimate,   // performance_estimate

    (int (*)())esdm_backend_t_wos_alloc,
    (int (*)())esdm_backend_t_wos_open,
    (int (*)())esdm_backend_t_wos_write,
    (int (*)())esdm_backend_t_wos_read,
    (int (*)())esdm_backend_t_wos_close,

    // Metadata Callbacks
    NULL, // lookup

    // ESDM Data Model Specific
    NULL, // container create
    NULL, // container retrieve
    NULL, // container update
    NULL, // container delete

    NULL, // dataset create
    NULL, // dataset retrieve
    NULL, // dataset update
    NULL, // dataset delete

    NULL,                                            // fragment create
    (int (*)())esdm_backend_t_wos_fragment_retrieve, // fragment retrieve
    (int (*)())esdm_backend_t_wos_fragment_update,   // fragment update
    (int (*)())esdm_backend_t_wos_fragment_delete,   // fragment delete
    (int (*)())esdm_backend_t_wos_fragment_mkfs,     // TODO
  },
};

esdm_backend_t *wos_backend_init(esdm_config_backend_t *config) {
  if (!config || !config->type || strcasecmp(config->type, "WOS") || !config->target) {
    DEBUG("Wrong configuration");
    return NULL;
  }

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  backend->data = ea_checked_malloc(sizeof(wos_backend_data_t));
  wos_backend_data_t *data = (wos_backend_data_t *)backend->data;
  if (!data)
    return NULL;

  // Initialize WOS backend
  if (esdm_backend_t_wos_init(config->target, backend))
    return NULL;

  if (config->performance_model) {
    esdm_backend_t_parse_dynamic_perf_model_lat_thp(config->performance_model, &data->perf_model);
    esdm_backend_t_start_dynamic_perf_model_lat_thp(&data->perf_model, backend, &wos_backend_performance_check);
  } else
    esdm_backend_t_reset_dynamic_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  json_t *elem;
  elem = json_object_get(config->backend, "target");
  data->target = json_string_value(elem);

  return backend;
}
