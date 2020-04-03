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
 * @brief A data backend to provide a DUMMY interface that doesn't perform any operation.
 */

#define _GNU_SOURCE /* See feature_test_macros(7) */


#include <dirent.h>
#include <errno.h>
#include <esdm-debug.h>
#include <esdm.h>
#include <fcntl.h>
#include <jansson.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dummy.h"
#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("DUMMY", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("DUMMY", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("DUMMY", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("DUMMY", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("DUMMY", "%s", fmt)


static int mkfs(esdm_backend_t *backend, int format_flags) {
  DEBUG("mkfs: backend dummy\n", "");

  return ESDM_SUCCESS;
}

static int fsck(esdm_backend_t* backend) {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;
  //ensure that we have a contiguous read buffer
  void* readBuffer = f->dataspace->stride ? malloc(f->bytes) : f->buf;

  if(f->dataspace->stride) {
    //data is not necessarily supposed to be contiguous in memory -> copy from contiguous dataspace
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(contiguousSpace, readBuffer, f->dataspace, f->buf);
    esdm_dataspace_destroy(contiguousSpace);
    free(readBuffer);
  }

  return ESDM_SUCCESS;
}

static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convenience
  int ret = ESDM_SUCCESS;
  //ensure that we have the data contiguously in memory
  void* writeBuffer = f->buf;
  if(f->dataspace->stride) {
    //data is not necessarily contiguous in memory -> copy to contiguous dataspace
    writeBuffer = malloc(f->bytes);
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(f->dataspace, f->buf, contiguousSpace, writeBuffer);
    esdm_dataspace_destroy(contiguousSpace);
  }
  // lazy assignment of ID
  if(f->id == NULL){
    f->id = malloc(24);
    eassert(f->id);
    ea_generate_id(f->id, 23);
  }

  //cleanup
  if(f->dataspace->stride) free(writeBuffer);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int dummy_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  dummy_backend_data_t *data = (dummy_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

static float dummy_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;

  dummy_backend_data_t *data = (dummy_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

static int fragment_delete(esdm_backend_t * backend, esdm_fragment_t *f){
  return ESDM_SUCCESS;
}

int dummy_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  dummy_backend_data_t* data = backend->data;
  free(data->config);  //TODO: Do we need to destruct this?
  free(data);
  free(backend);

  return 0;
}



///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
  ///////////////////////////////////////////////////////////////////////////////
  // NOTE: This serves as a template for the dummy plugin and is memcopied!    //
  ///////////////////////////////////////////////////////////////////////////////
  .name = "DUMMY",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = dummy_finalize,
    .performance_estimate = dummy_backend_performance_estimate,
    .estimate_throughput = dummy_backend_estimate_throughput,
    .fragment_create = NULL,
    .fragment_retrieve = fragment_retrieve,
    .fragment_update = fragment_update,
    .fragment_delete = fragment_delete,
    .fragment_metadata_create = NULL,
    .fragment_metadata_load = NULL,
    .fragment_metadata_free = NULL,
    .mkfs = mkfs,
    .fsck = fsck,
  },
};

// Two versions of this function!!!
//
// datadummy.c

esdm_backend_t *dummy_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "DUMMY") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = (esdm_backend_t *)malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  dummy_backend_data_t *data = malloc(sizeof(*data));
  backend->data = data;

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  return backend;
}
