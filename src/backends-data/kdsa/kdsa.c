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
 * @brief A data backend to provide Kove XPD KDSA compatibility.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <esdm-internal.h>


#include <kdsa.h>

#include "esdm-kdsa.h"

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("KDSA", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("KDSA", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("KDSA", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("KDSA", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("KDSA", "%s", fmt)


#define sprintfFragmentDir(path, f) (sprintf(path, "%s/%c%c/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2))
#define sprintfFragmentPath(path, f) (sprintf(path, "%s/%c%c/%s/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2, f->id))

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int mkfs(esdm_backend_t *backend, int format_flags) {
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;

  DEBUG("mkfs: backend->(void*)data->target = %s\n", data->target);

  const char *tgt = data->target;
  if (strlen(tgt) < 6) {
    WARNS("safety, tgt directory shall be longer than 6 chars");
    return ESDM_ERROR;
  }
  char path[PATH_MAX];
  struct stat sb = {0};
  int const ignore_err = format_flags & ESDM_FORMAT_IGNORE_ERRORS;

  if (format_flags & ESDM_FORMAT_DELETE) {
    printf("[mkfs] Removing %s\n", tgt);

    sprintf(path, "%s/README-ESDM.TXT", tgt);
    if (stat(path, &sb) == 0) {
    }else if(! ignore_err){
      printf("[mkfs] Error %s is not an ESDM directory\n", tgt);
      return ESDM_ERROR;
    }
  }

  if(! (format_flags & ESDM_FORMAT_CREATE)){
    return ESDM_SUCCESS;
  }
  if (stat(tgt, &sb) == 0) {
    if(! ignore_err){
      printf("[mkfs] Error %s exists already\n", tgt);
      return ESDM_ERROR;
    }
    printf("[mkfs] WARNING %s exists already\n", tgt);
  }

  printf("[mkfs] Creating %s\n", tgt);

  int ret = 0;
  // TODO
  if (ret != 0) {
    if(ignore_err){
      printf("[mkfs] WARNING couldn't create dir %s\n", tgt);
    }else{
      return ESDM_ERROR;
    }
  }

  sprintf(path, "%s/README-ESDM.TXT", tgt);
  char str[] = "This directory belongs to ESDM and contains various files that are needed to make ESDM work. Do not delete it until you know what you are doing.";
  //ret = write_data(path, str, strlen(str), 0);

  if (ret != 0) {
    if(ignore_err){
      printf("[mkfs] WARNING couldn't write %s\n", tgt);
    }else{
      return ESDM_ERROR;
    }
  }

  return ESDM_SUCCESS;
}

static int fsck() {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *f, json_t *metadata) {
  DEBUG_ENTER;

  // set data, options and tgt for convienience
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  const char *tgt = data->target;
  int ret = 0;

  return ret;
}


static int fragment_metadata_create(esdm_backend_t *backend, esdm_fragment_t *fragment, int len, char * md, int * out_size){
  DEBUG_ENTER;
  int size = 0;
  size = snprintf(md, len, "{}");
  *out_size = size;

  return 0;
}

static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convienience
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  const char *tgt = data->target;
  int ret = ESDM_SUCCESS;

  char path[PATH_MAX];
  // lazy assignment of ID
  if(f->id != NULL){
    sprintfFragmentPath(path, f);
    DEBUG("path: %s\n", path);
    // create data
    //int ret = entry_update(path, f->buf, f->bytes, 1);
    return ret;
  }
  f->id = malloc(21);
  eassert(f->id);
  // ensure that the fragment with the ID doesn't exist, yet
  //while(1){
  ea_generate_id(f->id, 20);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int kdsa_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

int kdsa_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
///////////////////////////////////////////////////////////////////////////////
// NOTE: This serves as a template for the posix plugin and is memcopied!    //
///////////////////////////////////////////////////////////////////////////////
.name = "KDSA",
.type = ESDM_MODULE_DATA,
.version = "0.0.1",
.data = NULL,
.callbacks = {
  NULL,                               // finalize
  kdsa_backend_performance_estimate, // performance_estimate
  NULL,
  fragment_retrieve,
  fragment_update,
  fragment_metadata_create,
  NULL,
  mkfs,
},
};


esdm_backend_t *kdsa_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "KDSA") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = (esdm_backend_t *)malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  backend->data = malloc(sizeof(kdsa_backend_data_t));
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  json_t *elem;
  elem = json_object_get(config->backend, "target");
  data->target = json_string_value(elem);

  DEBUG("Backend config: target=%s\n", data->target);

  return backend;
}
