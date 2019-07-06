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
 * @brief A data backend to provide POSIX compatibility.
 */

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <assert.h>
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

#include "posix.h"
#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("POSIX", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("POSIX", fmt, __VA_ARGS__)

#define sprintfFragmentDir(path, f) (sprintf(path, "%s/%c%c/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2))
#define sprintfFragmentPath(path, f) (sprintf(path, "%s/%c%c/%s/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2, f->id))

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Internal Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_retrieve(const char *path, void *buf, uint64_t size) {
  int status;
  struct stat sb;

  DEBUG("entry_retrieve(%s)\n", path);

  // write to non existing file
  int fd = open(path, O_RDONLY);
  // everything ok? read and close
  if (fd < 0) {
    return ESDM_ERROR;
  }
  int ret = read_check(fd, buf, size);
  close(fd);
  return ret;
}

static int entry_update(const char *path, void *buf, size_t len, int update_only) {
  DEBUG("entry_update(%s: %ld)\n", path, len);
  int flags;

  if(update_only){
    flags = O_TRUNC;
  }else{
    flags = O_CREAT | O_EXCL;
  }

  // write to non existing file
  int fd = open(path, O_WRONLY | flags, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
  if(fd < 0){
    return ESDM_ERROR;
  }
  printf("%s fd: %d\n", path, fd);
  int ret = write_check(fd, buf, len);
  close(fd);
  printf("END %lld\n", len);

  return ret;
}

static int entry_destroy(const char *path) {
  DEBUG_ENTER;

  int status;
  struct stat sb;

  DEBUG("entry_destroy(%s)\n", path);

  status = stat(path, &sb);
  if (status == -1) {
    perror("stat");
    return -1;
  }

  print_stat(sb);

  status = unlink(path);
  if (status == -1) {
    perror("unlink");
    return -1;
  }

  return 0;
}


static int mkfs(esdm_backend_t *backend, int format_flags) {
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;

  DEBUG("mkfs: backend->(void*)data->target = %s\n", data->target);

  const char *tgt = data->target;
  if (strlen(tgt) < 6) {
    return ESDM_ERROR;
  }
  char path[PATH_MAX];
  struct stat sb = {0};
  int const ignore_err = format_flags & ESDM_FORMAT_IGNORE_ERRORS;

  if (format_flags & ESDM_FORMAT_DELETE) {
    printf("[mkfs] Removing %s\n", tgt);

    sprintf(path, "%s/README-ESDM.TXT", tgt);
    if (stat(path, &sb) == 0) {
      posix_recursive_remove(tgt);
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

  int ret = mkdir(tgt, 0700);
  if (ret != 0) {
    if(ignore_err){
      printf("[mkfs] WARNING couldn't create dir %s\n", tgt);
    }else{
      return ESDM_ERROR;
    }
  }

  sprintf(path, "%s/README-ESDM.TXT", tgt);
  char str[] = "This directory belongs to ESDM and contains various files that are needed to make ESDM work. Do not delete it until you know what you are doing.";
  ret = entry_update(path, str, strlen(str), 0);
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
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  const char *tgt = data->target;

  // determine path to fragment
  char path[PATH_MAX];
  sprintfFragmentPath(path, f);
  DEBUG("path_fragment: %s", path);

  int ret = entry_retrieve(path, f->buf, f->bytes);
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
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  const char *tgt = data->target;

  char path[PATH_MAX];
  // lazy assignment of ID
  if(f->id == NULL){
    f->id = malloc(17);
    assert(f->id);
    // ensure that the fragment with the ID doesn't exist, yet
    while(1){
      ea_generate_id(f->id, 16);
      sprintfFragmentPath(path, f);

      struct stat sb;
      if (stat(path, &sb) == -1) {
        sprintfFragmentDir(path, f);
        if (stat(path, &sb) == -1) {
          int ret = mkdir_recursive(path);
          if (ret != 0 && errno != EEXIST) return ESDM_ERROR;
        }
        break;
      }
    }
  }

  sprintfFragmentPath(path, f);
  DEBUG("path: %s\n", path);

  // create data
  int ret = entry_update(path, f->buf, f->bytes, 0);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int posix_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

int posix_finalize(esdm_backend_t *backend) {
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
.name = "POSIX",
.type = SMD_DTYPE_DATA,
.version = "0.0.1",
.data = NULL,
.callbacks = {
NULL,                               // finalize
posix_backend_performance_estimate, // performance_estimate
NULL,
fragment_retrieve,
fragment_update,
fragment_metadata_create,
NULL,
mkfs,
},
};

// Two versions of this function!!!
//
// datadummy.c

esdm_backend_t *posix_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "POSIX") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = (esdm_backend_t *)malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  backend->data = (void *)malloc(sizeof(posix_backend_data_t));
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;

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
