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
 * @brief A data backend to provide optimized IME performance
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

#include <ime_native.h>

#include "ime.h"
#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("IME", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("IME", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("IME", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("IME", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("IME", "%s", fmt)


#define sprintfFragmentDir(path, f) (sprintf(path, "%s/%c/%c/%s/%c/%c", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2, f->id[0], f->id[1]))
#define sprintfFragmentPath(path, f) (sprintf(path, "%s/%c/%c/%s/%c/%c/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2, f->id[0], f->id[1], f->id+2))

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int write_check_ime(int fd, char *buf, size_t len) {
  while (len > 0) {
    ssize_t ret = ime_native_write(fd, buf, len);
    if (ret != -1) {
      buf = buf + ret;
      len -= ret;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        ESDM_ERROR_COM_FMT("IME", "write %s", strerror(errno));
        return 1;
      }
    }
  }
  return 0;
}

int read_check_ime(int fd, char *buf, size_t len) {
  while (len > 0) {
    ssize_t ret = ime_native_read(fd, buf, len);
    if (ret == 0) {
      return 1;
    } else if (ret != -1) {
      buf += ret;
      len -= ret;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        ESDM_ERROR_COM_FMT("IME", "read %s", strerror(errno));
        return 1;
      }
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Internal Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_retrieve(const char *path, void *buf, uint64_t size) {
  DEBUG("entry_retrieve(%s)", path);

  // write to non existing file
  int fd = ime_native_open(path, O_RDONLY, 0);
  // everything ok? read and close
  if (fd < 0) {
    WARN("error on opening file \"%s\": %s", path, strerror(errno));
    return ESDM_ERROR;
  }
  int ret = read_check_ime(fd, buf, size);
  ime_native_close(fd);
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
  int fd = ime_native_open(path, O_WRONLY | flags, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
  if(fd < 0){
    WARN("error on opening file: %s", strerror(errno));
    return ESDM_ERROR;
  }
  int ret = write_check_ime(fd, buf, len);
  ime_native_close(fd);

  return ret;
}

static int fragment_delete(esdm_backend_t * backend, esdm_fragment_t *f){
  DEBUG_ENTER;

  ime_backend_data_t *data = (ime_backend_data_t *)backend->data;
  const char *tgt = data->target;

  char path[PATH_MAX];
  if(f->id == NULL){
    return ESDM_ERROR;
  }
  sprintfFragmentPath(path, f);

  int ret = ime_native_unlink(path);
  if (ret == -1) {
    perror("unlink");
    return ESDM_ERROR;
  }
  sprintfFragmentDir(path, f);
  ret = ime_native_rmdir(path);
  if (ret == -1) {
    perror("rmdir");
    return ESDM_ERROR;
  }

  return ESDM_SUCCESS;
}

static int mkfs(esdm_backend_t *backend, int format_flags) {
  ime_backend_data_t *data = (ime_backend_data_t *)backend->data;

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
    if (ime_native_stat(path, &sb) == 0) {
      if(posix_recursive_remove(tgt)) { // FIXME XXX
        fprintf(stderr, "[mkfs] Error removing ESDM directory at \"%s\"\n", tgt);
        return ESDM_ERROR;
      }
    }else if(! ignore_err){
      printf("[mkfs] Error %s is not an ESDM directory\n", tgt);
      return ESDM_ERROR;
    }
  }

  if(! (format_flags & ESDM_FORMAT_CREATE)){
    return ESDM_SUCCESS;
  }
  if (ime_native_stat(tgt, &sb) == 0) {
    if(! ignore_err){
      printf("[mkfs] Error %s exists already\n", tgt);
      return ESDM_ERROR;
    }
    printf("[mkfs] WARNING %s exists already\n", tgt);
  }

  printf("[mkfs] Creating %s\n", tgt);

  int ret = ime_native_mkdir(tgt, 0700);
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

static int fsck(esdm_backend_t* backend) {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convienience
  ime_backend_data_t *data = (ime_backend_data_t *)backend->data;
  const char *tgt = data->target;

  // determine path to fragment
  char path[PATH_MAX];
  sprintfFragmentPath(path, f);
  DEBUG("path_fragment: %s", path);

  //ensure that we have a contiguous read buffer
  void* readBuffer = f->dataspace->stride ? malloc(f->bytes) : f->buf;

  int ret = entry_retrieve(path, readBuffer, f->bytes);

  if(f->dataspace->stride) {
    //data is not necessarily supposed to be contiguous in memory -> copy from contiguous dataspace
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(contiguousSpace, readBuffer, f->dataspace, f->buf);
    esdm_dataspace_destroy(contiguousSpace);
    free(readBuffer);
  }

  return ret;
}

static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convenience
  ime_backend_data_t *data = (ime_backend_data_t *)backend->data;
  const char *tgt = data->target;
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

  char path[PATH_MAX];
  // lazy assignment of ID
  if(f->id != NULL){
    sprintfFragmentPath(path, f);
    DEBUG("path: %s\n", path);
    // create data
    ret = entry_update(path, writeBuffer, f->bytes, 1);
  } else {
    f->id = malloc(24);
    eassert(f->id);
    // ensure that the fragment with the ID doesn't exist, yet
    while(1){
      ea_generate_id(f->id, 23);
      struct stat sb;
      sprintfFragmentDir(path, f);
      if (ime_native_stat(path, &sb) == -1) {
        if (mkdir_recursive(path) != 0 && errno != EEXIST) { // FIXME XXX
          WARN("error on creating directory \"%s\": %s", path, strerror(errno));
          ret = ESDM_ERROR;
          break;
        }
      }
      sprintfFragmentPath(path, f);
      int fd = ime_native_open(path, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
      if(fd < 0){
        if(errno == EEXIST){
          continue;
        }
        WARN("error on creating file \"%s\": %s", path, strerror(errno));
        ret = ESDM_ERROR;
        break;
      }
      //write the data
      ret = write_check_ime(fd, writeBuffer, f->bytes);
      ime_native_close(fd);
      break;
    }
  }

  //cleanup
  if(f->dataspace->stride) free(writeBuffer);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int ime_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  ime_backend_data_t *data = (ime_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

static float ime_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;

  ime_backend_data_t *data = (ime_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

int ime_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  ime_backend_data_t* data = backend->data;
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
  // NOTE: This serves as a template for the ime plugin and is memcopied!    //
  ///////////////////////////////////////////////////////////////////////////////
  .name = "IME",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = ime_finalize,
    .performance_estimate = ime_backend_performance_estimate,
    .estimate_throughput = ime_backend_estimate_throughput,
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

esdm_backend_t *ime_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "IME") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = (esdm_backend_t *)malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  ime_backend_data_t *data = malloc(sizeof(*data));
  backend->data = data;

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
