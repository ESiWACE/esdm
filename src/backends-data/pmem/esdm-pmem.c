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
 * @brief A data backend to provide PMEM (pmem.io) support.
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
#include <pthread.h>

#include <esdm-internal.h>

#include <libpmem.h>

#include "esdm-pmem.h"

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("PMEM", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("PMEM", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("PMEM", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("PMEM", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("PMEM", "%s", fmt)

#define ERROR(fmt, ...) ESDM_ERROR_COM_FMT("PMEM", fmt, __VA_ARGS__)
#define ERRORS(fmt) ESDM_ERROR_COM_FMT("PMEM", "%s", fmt)

#define WARN_STRERR(fmt, ...) WARN(fmt ": %s", __VA_ARGS__, strerror(errno));
#define WARN_CHECK_RET(ret, fmt, ...) if(ret != 0){ WARN(fmt ": %s", __VA_ARGS__, strerror(errno)); }

#define sprintfFragmentDir(path, f) (sprintf(path, "%s/%c%c/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2))
#define sprintfFragmentPath(path, f) (sprintf(path, "%s/%c%c/%s/%s", tgt, f->dataset->id[0], f->dataset->id[1], f->dataset->id+2, f->id))

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__aarch64__)
// TODO: This might be general enough to provide the functionality for any system
// regardless of processor type given we aren't worried about thread/process migration.
// Test on Intel systems and see if we can get rid of the architecture specificity
// of the code.
static unsigned long GetProcessorAndCore(int *chip, int *core){
    return syscall(SYS_getcpu, core, chip, NULL);
}
// TODO: Add in AMD function
#else
// If we're not on an ARM processor assume we're on an intel processor and use the
// rdtscp instruction.
static unsigned long GetProcessorAndCore(int *chip, int *core){
    unsigned long a,d,c;
    __asm__ volatile("rdtscp" : "=a" (a), "=d" (d), "=c" (c));
    *chip = (c & 0xFFF000)>>12;
    *core = c & 0xFFF;
    return ((unsigned long)a) | (((unsigned long)d) << 32);;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Internal Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_retrieve(const char *path, void *buf, uint64_t size) {
  DEBUG("entry_retrieve(%s)", path);

  size_t mapped_len;
  int * fd = pmem_map_file(path, 0, 0, 0, &mapped_len, NULL);
  if(fd == NULL){
    WARN("error on opening file (%s): %s", path, strerror(errno));
    return ESDM_ERROR;
  }
  if(mapped_len != size){
    WARN("fragment size of file is wrong (%s): %zu", path, size);
    pmem_unmap(fd, mapped_len);
    return ESDM_ERROR;
  }
  memcpy(buf, fd, size);
  pmem_unmap(fd, size);
  return ESDM_SUCCESS;
}


static int entry_update(const char *path, void *buf, size_t len, int update_only) {
  DEBUG("entry_update(%s: %ld)\n", path, len);
  int flags = 0;
  if(! update_only){
    flags = PMEM_FILE_CREATE | PMEM_FILE_EXCL;
  }

  int * fd = pmem_map_file(path, len, flags, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH, NULL, NULL);
  if(fd == NULL){
    WARN("error on opening file: %s", strerror(errno));
    return ESDM_ERROR;
  }
  //write the data
  pmem_memcpy_persist(fd, buf, len);
  pmem_unmap(fd, len);

  return ESDM_SUCCESS;
}

static int fragment_delete(esdm_backend_t * backend, esdm_fragment_t *f){
  DEBUG_ENTER;

  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;
  const char *tgt = data->target;

  char path[PATH_MAX];
  if(f->id == NULL){
    return ESDM_ERROR;
  }
  sprintfFragmentPath(path, f);

  int ret = unlink(path);
  if (ret == -1) {
    perror("unlink");
    return ESDM_ERROR;
  }
  sprintfFragmentDir(path, f);
  ret = rmdir(path);
  if (ret == -1) {
    perror("rmdir");
    return ESDM_ERROR;
  }

  return ESDM_SUCCESS;
}

static int mkfs(esdm_backend_t *backend, int format_flags) {
  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;

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
      if(posix_recursive_remove(tgt)) {
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

static int fsck(esdm_backend_t* backend) {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convienience
  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;
  const char *tgt = data->target;

  // determine path to fragment
  char path[PATH_MAX];
  sprintfFragmentPath(path, f);
  DEBUG("path_fragment: %s", path);

  //ensure that we have a contiguous read buffer
  void* readBuffer = f->dataspace->stride ? ea_checked_malloc(f->bytes) : f->buf;

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

  // set data, options and tgt for convienience
  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;
  const char *tgt = data->target;
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

  char path[PATH_MAX];
  // lazy assignment of ID
  if(f->id != NULL){
    sprintfFragmentPath(path, f);
    DEBUG("path: %s\n", path);
    // create data
    ret = entry_update(path, writeBuffer, f->bytes, 1);
  } else {
    // ensure that the fragment with the ID doesn't exist, yet
    while(1){
      f->id = ea_make_id(ESDM_ID_LENGTH);
      struct stat sb;
      sprintfFragmentDir(path, f);
      if (stat(path, &sb) == -1) {
        if (mkdir_recursive(path) != 0 && errno != EEXIST) {
          WARN("error on creating directory \"%s\": %s", path, strerror(errno));
          ret = ESDM_ERROR;
          break;
        }
      }
      sprintfFragmentPath(path, f);
      int * fd = pmem_map_file(path, f->bytes, PMEM_FILE_CREATE | PMEM_FILE_EXCL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH, NULL, NULL);
      if(fd == NULL){
        if(errno == EEXIST){
          free(f->id);  //we'll make a new ID
          continue;
        }
        WARN("error on creating file \"%s\": %s", path, strerror(errno));
        ret = ESDM_ERROR;
        break;
      }
      //write the data
      pmem_memcpy_persist(fd, writeBuffer, f->bytes);
      pmem_unmap(fd, f->bytes);

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

static int pmem_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

static float pmem_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;

  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

int pmem_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
  ///////////////////////////////////////////////////////////////////////////////
  // NOTE: This serves as a template for the pmem plugin and is memcopied!    //
  ///////////////////////////////////////////////////////////////////////////////
  .name = "POSIX",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = NULL,                               // finalize
    .performance_estimate = pmem_backend_performance_estimate, // performance_estimate
    .estimate_throughput = pmem_backend_estimate_throughput,
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

esdm_backend_t *pmem_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "PMEM") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  backend->data = ea_checked_malloc(sizeof(pmem_backend_data_t));
  pmem_backend_data_t *data = (pmem_backend_data_t *)backend->data;

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  json_t *elem;
  elem = json_object_get(config->backend, "target");
  char const * tgt = json_string_value(elem);

  int socket;
  int core;
  int ret = GetProcessorAndCore(& socket, & core);
  data->target = ea_checked_malloc(strlen(tgt) + 3);
  sprintf((char*)data->target, "%s%d/esdm", tgt, socket);
  DEBUG("Backend config: socket=%d core=%d target=%s\n", socket, core, data->target);

  return backend;
}
