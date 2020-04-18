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
 * @brief Entry point for ESDM API Implementation
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esdm-internal.h>
// TODO: Decide on initialization mechanism.
static bool is_initialized = false;

#define ESDM_INSTANCE_INITIALIZERS \
  .procs_per_node = 1,\
  .total_procs = 1,\
  .config = NULL

esdm_instance_t esdm = {ESDM_INSTANCE_INITIALIZERS};

esdm_status esdm_set_procs_per_node(int procs) {
  eassert(procs > 0);
  eassert(!is_initialized);

  esdm.procs_per_node = procs;
  return ESDM_SUCCESS;
}

esdm_status esdm_set_total_procs(int procs) {
  eassert(procs > 0);
  eassert(!is_initialized);

  esdm.total_procs = procs;
  return ESDM_SUCCESS;
}

esdm_status esdm_load_config_str(const char *str) {
  eassert(str != NULL);
  eassert(!is_initialized);
  eassert(!esdm.config);

  esdm.config = esdm_config_init_from_str(str);
  return esdm.config ? ESDM_SUCCESS : ESDM_ERROR;
}

esdm_status esdm_dataset_get_dataspace(esdm_dataset_t *dset, esdm_dataspace_t **out_dataspace) {
  eassert(dset != NULL);
  *out_dataspace = dset->dataspace;
  eassert(*out_dataspace != NULL);
  return ESDM_SUCCESS;
}

int esdm_is_initialized(){
  return is_initialized;
}

esdm_status esdm_init() {
  ESDM_DEBUG("Init");

  if (!is_initialized) {
    if(atexit(esdmI_log_dump) != 0){
      ESDM_ERROR("Could not register log reporter");
    }
    char * str = getenv("ESDM_LOGLEVEL_BUFFER");
    if(str){
      int loglevel = atoi(str);
      ESDM_DEBUG_COM_FMT("ESDM", "Setting buffer debuglevel to %d", loglevel);
      esdm_loglevel_buffer(loglevel);
    }
    str = getenv("ESDM_LOGLEVEL");
    if(str){
      int loglevel = atoi(str);
      ESDM_DEBUG_COM_FMT("ESDM", "Setting debuglevel to %d", loglevel);
      esdm_loglevel(loglevel);
    }

    // find configuration
    if (! esdm.config){
      if(esdm_config_init(&esdm) == NULL){
        return ESDM_ERROR;
      }
    }

    // optional modules (e.g. data and metadata backends)
    esdm_modules_init(&esdm);

    // core components
    esdm_layout_init(&esdm);
    esdm_performance_init(&esdm);
    esdm_scheduler_init(&esdm);

    ESDM_DEBUG_COM_FMT("ESDM", " esdm = {config = %p, modules = %p, scheduler = %p, layout = %p, performance = %p}\n",
    (void *)esdm.config,
    (void *)esdm.modules,
    (void *)esdm.scheduler,
    (void *)esdm.layout,
    (void *)esdm.performance);

    is_initialized = true;

    ESDM_DEBUG("ESDM initialized and ready!");
  }

  return ESDM_SUCCESS;
}

esdm_status esdm_mkfs(int format_flags, data_accessibility_t target) {
  if (!is_initialized) {
    return ESDM_ERROR;
  }
  int ret;
  int ret_final = ESDM_SUCCESS;
  if (esdm.modules->metadata_backend->config->data_accessibility == target) {
    ret = esdm.modules->metadata_backend->callbacks.mkfs(esdm.modules->metadata_backend, format_flags);
    if (ret != ESDM_SUCCESS) {
      ret_final = ret;
    }
  }

  for (int i = 0; i < esdm.modules->data_backend_count; i++) {
    if (esdm.modules->data_backends[i]->config->data_accessibility == target) {
      ret = esdm.modules->data_backends[i]->callbacks.mkfs(esdm.modules->data_backends[i], format_flags);
      if (ret != ESDM_SUCCESS) {
        ret_final = ret;
      }
    }
  }
  return ret_final;
}

esdm_status esdm_finalize() {
  ESDM_DEBUG(__func__);

  // ESDM data data structures that require proper cleanup..
  // in particular this effects data and cache state which is not yet persistent

  esdm_scheduler_finalize(&esdm);
  esdm_performance_finalize(&esdm);
  esdm_layout_finalize(&esdm);
  esdm_modules_finalize(&esdm);
  esdm_config_finalize(&esdm);

  esdm_log_on_exit(0);

  esdm = (esdm_instance_t){ESDM_INSTANCE_INITIALIZERS};
  is_initialized = false;

  return ESDM_SUCCESS;
}

esdm_status esdm_write(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *space, esdm_dataspace_t *memspace) {
  ESDM_DEBUG(__func__);
  eassert(dataset);
  eassert(buf);
  eassert(space);

  if(space->dims == 0){
    // this is a workaround to deal with 0 dimensional data
    space->dims = 1;
    space->size[0] = 1;
    space->offset[0] = 0;
    int ret = esdm_scheduler_write_blocking(&esdm, dataset, buf,  space,memspace, false);
    space->dims = 0;
    return ret;
  }

  return esdm_scheduler_write_blocking(&esdm, dataset, buf, space, memspace, false);
}

esdm_status esdmI_readWithFillRegion(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *space, esdm_dataspace_t *memspace, esdmI_hypercubeSet_t** out_fillRegion) {
  ESDM_DEBUG("");
  eassert(dataset);
  eassert(buf);
  eassert(space);

  if(space->dims == 0){
    // this is a workaround to deal with 0 dimensional data
    space->dims = 1;
    space->size[0] = 1;
    space->offset[0] = 0;
    int ret = esdm_scheduler_read_blocking(&esdm, dataset, buf, space, memspace, out_fillRegion, false);
    space->dims = 0;
    return ret;
  }

  return esdm_scheduler_read_blocking(&esdm, dataset, buf, space, memspace, out_fillRegion, false);
}

esdm_status esdm_read(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *space, esdm_dataspace_t *memspace) {
  return esdmI_readWithFillRegion(dataset, buf, space, memspace, NULL);
}

esdm_status esdm_sync() {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}

int esdm_container_get_mode_flags(esdm_container_t *c){
  return c->mode_flags;
}

esdm_status esdm_dataset_set_fill_value(esdm_dataset_t *d, void const * value){
  eassert(d);
  if(d->fill_value){
    smd_attr_destroy(d->fill_value);
    d->fill_value = NULL;
  }
  if(value != NULL){
    d->fill_value = smd_attr_new("fill-value", d->dataspace->type, value);
  }
  return ESDM_SUCCESS;
}

int esdm_dataset_is_fill_value_set(esdm_dataset_t *d){
  eassert(d);
  return d->fill_value != NULL;
}

esdm_status esdm_dataset_get_fill_value(esdm_dataset_t *d, void * value){
  eassert(d);
  eassert(value);
  if(! d->fill_value){
    return ESDM_ERROR;
  }
  smd_attr_copy_value(d->fill_value, value);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_change_name(esdm_dataset_t *d, char const * new_name){
  eassert(d);
  eassert(new_name);
  //FIXME Memory leak: the old name needs to be freed
  d->name = strdup(new_name);
  d->status = ESDM_DATA_DIRTY;
  return ESDM_SUCCESS;
}

esdm_status esdm_read_stream(esdm_dataset_t *d, esdm_dataspace_t *space, void * user_ptr, esdm_stream_func_t stream_func, esdm_reduce_func_t reduce_func)
{
  // TODO emulation function for now.
  uint64_t size = esdm_dataspace_size(space);
  void * buf = malloc(size);
  esdm_status ret = esdmI_readWithFillRegion(d, buf, space, NULL, NULL);  //FIXME: Don't ignore the return value.
  void * intermediate = stream_func(space, buf, user_ptr, d->fill_value);
  if(reduce_func){
    reduce_func(space, user_ptr, intermediate);
  }
  free(buf);

  return ret;
}

esdm_statistics_t esdm_read_stats() { return esdm.readStats; }

esdm_statistics_t esdm_write_stats() { return esdm.writeStats; }

esdm_config_t* esdmI_getConfig() { return esdm.config; }
