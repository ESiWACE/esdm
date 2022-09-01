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

#include <esdm-stream.h>

#include "posix.h"
#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("POSIX", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("POSIX", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("POSIX", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("POSIX", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("POSIX", "%s", fmt)

#define ESDM_POSIX_ID_LENGTH 10
#define sprintfFragmentDir(path, f) (sprintf(path, "%s/%c/%c", tgt, f->id[0], f->id[1]))
#define sprintfFragmentPath(path, f) (sprintf(path, "%s/%c/%c/%c%c%c%c%c%c%c%c", tgt, f->id[0], f->id[1], f->id[2], f->id[3], f->id[4], f->id[5], f->id[6], f->id[7], f->id[8], f->id[9]))

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Internal Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
    WARN("error on opening file: %s", strerror(errno));
    return ESDM_ERROR;
  }
  int ret = ea_write_check(fd, buf, len);
  close(fd);

  return ret;
}

static int fragment_delete(esdm_backend_t * backend, esdm_fragment_t *f){
  DEBUG_ENTER;

  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  const char *tgt = data->config->target;

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
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;

  DEBUG("mkfs: backend->(void*)data->config->target = %s\n", data->config->target);

  const char *tgt = data->config->target;
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
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  const char *tgt = data->config->target;

  // determine path to fragment
  char path[PATH_MAX];
  sprintfFragmentPath(path, f);
  DEBUG("retrieve path_fragment: %s", path);

  void* readBuffer;
  size_t size;
  int ret;
  bool needUnpack = estream_mem_unpack_fragment_param(f, & readBuffer, & size);
  int fd = open(path, O_RDONLY);
  if (fd >= 0) {
    uint64_t epos = *(uint64_t*) f->backend_md;
    off_t pos = lseek(fd, epos, SEEK_SET);
    if (epos != pos){
      WARN("Cannot seek to the expected position: %s", strerror(errno));
      return ESDM_ERROR;
    }
    ret = ea_read_check(fd, readBuffer, size);
    close(fd);
  }else{
    WARN("error on opening file \"%s\": %s", path, strerror(errno));
    return ESDM_ERROR;
  }
  if(needUnpack){
    ret = estream_mem_unpack_fragment(f, readBuffer, size);
  }

  return ret;
}

static int create_posix_id(posix_backend_data_t * b, esdm_fragment_t * f, const char *tgt, int * out_fd){
  char path[PATH_MAX];
  // piggyback on previous fragment
  if(b->openfd){
    uint64_t offset = lseek(b->openfd, 0, SEEK_END);
    f->id = ea_checked_malloc(ESDM_POSIX_ID_LENGTH + 11);
    sprintf(f->id, "%s%010llu", b->open_fragment, (long long unsigned)  offset);
    *out_fd = b->openfd;    
    f->backend_md = ea_checked_malloc(sizeof(uint64_t));
    *(uint64_t*)f->backend_md = offset;
    return ESDM_SUCCESS;
  }
  
  // ensure that the fragment with the ID doesn't exist, yet
  while(1){
    f->id = ea_make_id(ESDM_POSIX_ID_LENGTH);
    struct stat sb;
    sprintfFragmentDir(path, f);
    if (stat(path, &sb) == -1) {
      if (mkdir_recursive(path) != 0 && errno != EEXIST) {
        WARN("error on creating directory \"%s\": %s", path, strerror(errno));
        return ESDM_ERROR;
      }
    }
    sprintfFragmentPath(path, f);
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    if(fd < 0){
      if(errno == EEXIST){
        free(f->id);  //we'll make a new ID
        continue;
      }
      WARN("error on creating file \"%s\": %s", path, strerror(errno));
      return ESDM_ERROR;
    }
    *out_fd = fd;
    b->openfd = fd;
    b->open_fragment = strdup(f->id);
    char * id = ea_checked_malloc(ESDM_POSIX_ID_LENGTH + 11);
    sprintf(id, "%s%010llu", b->open_fragment, 0llu);
    f->id = id;
    f->backend_md = ea_checked_malloc(sizeof(uint64_t));
    *(uint64_t*)f->backend_md = 0;
    return ESDM_SUCCESS;
  }
}

typedef struct{
  int fd;
} posix_stream_t;

static int fragment_write_stream_blocksize(esdm_backend_t * b, estream_write_t * state, void * c_buf, size_t c_off, uint64_t c_size){
  int ret;
  esdm_fragment_t * f = state->fragment;
  posix_backend_data_t *data = (posix_backend_data_t *) b->data;
  const char *tgt = data->config->target;
  posix_stream_t * s = (posix_stream_t*) state->backend_state;

  if(c_off == 0){
    // start stream
    s = ea_checked_malloc(sizeof(posix_stream_t));
    // lazy assignment of ID
    if(f->id != NULL){
      char path[PATH_MAX];
      sprintfFragmentPath(path, f);
      DEBUG("path: %s\n", path);
      s->fd = open(path, O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
      if(s->fd < 0){
        WARN("error on opening file: %s", strerror(errno));
        return ESDM_ERROR;
      }
    } else {
      ret = create_posix_id(data, f, tgt, & s->fd);
      if(ret != ESDM_SUCCESS){
        free(s);
        return ret;
      }
    }
    state->backend_state = s;
  }

  assert(c_off + c_size <= f->bytes);

  //write the data
  ret = ea_write_check(s->fd, c_buf, c_size);

  if(c_off + c_size == f->bytes){
    // done with streaming
    close(s->fd);
    free(s);
  }
  return ESDM_SUCCESS;
}


static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convenience
  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  const char *tgt = data->config->target;
  int ret = ESDM_SUCCESS;

  void * buff = NULL;
  size_t buff_size;
  ret = estream_mem_pack_fragment(f, & buff, & buff_size);
  if(ret != ESDM_SUCCESS) return ret;

  // lazy assignment of ID
  if(f->id != NULL){
    char path[PATH_MAX];
    sprintfFragmentPath(path, f);
    // create data
    ret = entry_update(path, buff, buff_size, 1);
  } else {
    int fd;
    ret = create_posix_id(data, f, tgt, & fd);
    if(ret == ESDM_SUCCESS){
      uint64_t epos = *(uint64_t*) f->backend_md;
      off_t pos = lseek(fd, epos, SEEK_SET);
      if (epos != pos){
        WARN("Cannot seek to the expected position: %s", strerror(errno));
        return ESDM_ERROR;
      }
      ret = ea_write_check(fd, buff, buff_size);
    }
  }

  // cleanup of estream
  if(buff != f->buf) free(buff);

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

static float posix_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;

  posix_backend_data_t *data = (posix_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

int posix_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  posix_backend_data_t* data = backend->data;
  
  if(data->openfd){
    close(data->openfd);
  }
  
  free(data->config);  //TODO: Do we need to destruct this?
  free(data);
  free(backend);

  return 0;
}

static void * fragment_metadata_load(esdm_backend_t * b, esdm_fragment_t *f, json_t *md){
  eassert(f->id);
  uint64_t * offset = ea_checked_malloc(sizeof(uint64_t));
  long long unsigned o;
  sscanf(f->id + ESDM_POSIX_ID_LENGTH, "%llu", &o);
  *offset = o;
  return offset;
}

static int fragment_metadata_free(esdm_backend_t * b, void * f){
  free(f);
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
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = posix_finalize,
    .performance_estimate = posix_backend_performance_estimate,
    .estimate_throughput = posix_backend_estimate_throughput,
    .fragment_create = NULL,
    .fragment_retrieve = fragment_retrieve,
    .fragment_update = fragment_update,
    .fragment_delete = fragment_delete,
    .fragment_metadata_create = NULL,
    .fragment_metadata_load = fragment_metadata_load,
    .fragment_metadata_free = fragment_metadata_free,
    .mkfs = mkfs,
    .fsck = fsck,
    .fragment_write_stream_blocksize = fragment_write_stream_blocksize
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

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  posix_backend_data_t *data = ea_checked_malloc(sizeof(*data));
  memset(data, 0, sizeof(posix_backend_data_t));
  backend->data = data;

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  DEBUG("Backend config: target=%s\n", config->target);

  return backend;
}
