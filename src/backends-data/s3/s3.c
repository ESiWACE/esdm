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

#include "s3.h"
#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("S3", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("S3", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("S3", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("S3", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("S3", "%s", fmt)


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
  void* readBuffer = f->dataspace->stride ? ea_checked_malloc(f->bytes) : f->buf;

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
    writeBuffer = ea_checked_malloc(f->bytes);
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(f->dataspace, f->buf, contiguousSpace, writeBuffer);
    esdm_dataspace_destroy(contiguousSpace);
  }
  // lazy assignment of ID
  if(f->id == NULL){
    f->id = ea_checked_malloc(24);
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

static int s3_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  s3_backend_data_t *data = (s3_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

static float s3_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;

  s3_backend_data_t *data = (s3_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

static int fragment_delete(esdm_backend_t * backend, esdm_fragment_t *f){
  return ESDM_SUCCESS;
}

int s3_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  s3_backend_data_t* data = backend->data;
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
  .name = "S3",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = s3_finalize,
    .performance_estimate = s3_backend_performance_estimate,
    .estimate_throughput = s3_backend_estimate_throughput,
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

esdm_backend_t *s3_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "S3") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  s3_backend_data_t *data = ea_checked_malloc(sizeof(*data));
  backend->data = data;

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  return backend;
}
