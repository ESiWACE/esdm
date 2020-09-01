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
 * @brief A metadata backend on top of a POSIX compatible filesystem.
 */

#define _GNU_SOURCE /* See feature_test_macros(7) */


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
#include <time.h>

#include "md-posix.h"
#include <esdm-internal.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("METADUMMY", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("METADUMMY", fmt, __VA_ARGS__)

#define sprintfDatasetDir(path, d) (sprintf(path, "%s/datasets/%c%c", tgt, d->id[0], d->id[1]))
#define sprintfDatasetMd(path, d) (sprintf(path, "%s/datasets/%c%c/%s.md", tgt, d->id[0], d->id[1], d->id + 2))

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Internal Helpers  //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_create(const char *path, char * const json, int size) {
  DEBUG_ENTER;

  // int status;
  // struct stat sb;

  DEBUG("entry_create(%s - %s)\n", path, json);

  // ENOENT => allow to create
  // write to non existing file
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
  // everything ok? write and close
  if (fd < 0) {
    return 1;
  }
  if ( json != NULL) {
    int ret = ea_write_check(fd, json, size);
    close(fd);
    return ret;
  }
  close(fd);
  return ESDM_SUCCESS;
}

static int entry_update(const char *path, void *buf, size_t len) {
  DEBUG_ENTER;

  DEBUG("entry_update(%s)\n", path);

  // write to non existing file
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);

  // everything ok? write and close
  if (fd != -1) {
    // write some metadata
    ea_write_check(fd, buf, len);
    close(fd);
  }

  return 0;
}


static int mkfs(esdm_md_backend_t *backend, int format_flags) {
  DEBUG_ENTER;
  // use target directory from backend configuration
  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;

  // Enforce min target length?
  const char *tgt = options->target;
  if (strlen(tgt) < 6) {
    printf("[mkfs] error, the target name is to short (< 6 characters)!\n");
    return ESDM_ERROR;
  }

  struct stat sb;
  char path[PATH_MAX];
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

  sprintf(path, "%s/containers", tgt);
  ret = mkdir(path, 0700);
  if (ret != 0) {
    if(ignore_err){
      printf("[mkfs] WARNING couldn't create dir %s\n", tgt);
    }else{
      return ESDM_ERROR;
    }
  }

  sprintf(path, "%s/README-ESDM.TXT", tgt);
  char str[] = "This directory belongs to ESDM and contains various files that are needed to make ESDM work. Do not delete it until you know what you are doing.";
  ret = entry_create(path, str, strlen(str));
  if (ret != 0) {
    if(ignore_err){
      printf("[mkfs] WARNING couldn't write %s\n", tgt);
    }else{
      return ESDM_ERROR;
    }
  }

  return ESDM_SUCCESS;
}

static int fsck(esdm_md_backend_t* backend) {
  DEBUG_ENTER;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Container Helpers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int container_create(esdm_md_backend_t *backend, esdm_container_t *container, int allow_overwrite) {
  DEBUG_ENTER;

  char path[PATH_MAX];

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;


  // create subdirectory if needed; find the last slash
  for(int i = strlen(container->name) - 1; i > 1; i--){
    if(container->name[i] == '/'){
      char * dir = ea_checked_strdup(container->name);
      dir[i] = 0;
      sprintf(path, "%s/containers/%s", tgt, dir);
      free(dir);
      mkdir_recursive(path);
      break;
    }
  }

  sprintf(path, "%s/containers/%s.md", tgt, container->name);

  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
  if(fd < 0){
    if( allow_overwrite && errno == EEXIST ){
      fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
      // TODO cleanup the old content of the container
      if(fd < 0){
        return ESDM_ERROR;
      }
      close(fd);
      return ESDM_SUCCESS;
    }

    return ESDM_ERROR;
  }
  close(fd);

  return ESDM_SUCCESS;
}

static int container_remove(esdm_md_backend_t *backend, esdm_container_t *c){
  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  char path_metadata[PATH_MAX];
  DEBUG("tgt: %p\n", tgt);
  sprintf(path_metadata, "%s/containers/%s.md", tgt, c->name);

  if(unlink(path_metadata) == 0){
    return ESDM_SUCCESS;
  }
  return ESDM_ERROR;
}

static int dataset_remove(esdm_md_backend_t * backend, esdm_dataset_t *d){
  char path_metadata[PATH_MAX];
  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  DEBUG("tgt: %p\n", tgt);

  sprintfDatasetMd(path_metadata, d);
  if(unlink(path_metadata) == 0){
    return ESDM_SUCCESS;
  }
  return ESDM_ERROR;
}

static int container_commit(esdm_md_backend_t *backend, esdm_container_t *container, char * json, int md_size) {
  DEBUG_ENTER;

  char path_metadata[PATH_MAX];

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  DEBUG("tgt: %p\n", tgt);

  sprintf(path_metadata, "%s/containers/%s.md", tgt, container->name);

  // create metadata entry
  esdm_status ret = entry_create(path_metadata, json, md_size);
  return ret;
}

static int container_retrieve(esdm_md_backend_t *backend, esdm_container_t *container, char ** out_json, int * out_size) {
  DEBUG_ENTER;
  int ret;
  char path_metadata[PATH_MAX];

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  sprintf(path_metadata, "%s/containers/%s.md", tgt, container->name);

  struct stat statbuf;
  ret = stat(path_metadata, &statbuf);
  if (ret != 0) return ESDM_ERROR;
  off_t len = statbuf.st_size + 1;

  int fd = open(path_metadata, O_RDONLY);
  if (fd < 0) return ESDM_ERROR;
  char * json = ea_checked_malloc(len);
  ret = ea_read_check(fd, json, statbuf.st_size);
  close(fd);
  json[statbuf.st_size] = 0;
  if (ret != 0){
    return ESDM_ERROR;
  }
  *out_json = json;
  *out_size = statbuf.st_size;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Dataset Helpers ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int dataset_create(esdm_md_backend_t * backend, esdm_dataset_t *d){
  DEBUG_ENTER;
  char path_dataset[PATH_MAX];
  eassert(backend);
  eassert(d);

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  while(1){
    // TODO fix race condition with the file creation here
    d->id = ea_make_id(ESDM_ID_LENGTH);

    // create directory for datsets
    sprintfDatasetMd(path_dataset, d);
    struct stat sb;
    if (stat(path_dataset, &sb) == -1) {
      sprintfDatasetDir(path_dataset, d);
      if (stat(path_dataset, &sb) == -1) {
        int ret = mkdir_recursive(path_dataset);
        if (ret != 0 && errno != EEXIST) return ESDM_ERROR;
      }
      return ESDM_SUCCESS;
    }

    free(d->id);  //we'll make a new ID
  }
}

static int dataset_commit(esdm_md_backend_t *backend, esdm_dataset_t *dataset, char * json, int md_size) {
  DEBUG_ENTER;

  char path_metadata[PATH_MAX];
  char path_dataset[PATH_MAX];
  struct stat sb;

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  DEBUG("tgt: %p\n", tgt);

  sprintfDatasetMd(path_metadata, dataset);
  sprintfDatasetDir(path_dataset, dataset);
  // create directory for datsets
  if (stat(path_dataset, &sb) == -1) {
    int ret = mkdir_recursive(path_dataset);
    if (ret != 0) return ESDM_ERROR;
  }

  // create metadata entry
  esdm_status ret = entry_create(path_metadata, json, md_size);
  return ret;
}

static int dataset_retrieve(esdm_md_backend_t *backend, esdm_dataset_t *d, char ** out_json, int * out_size) {
  DEBUG_ENTER;
  int ret;
  char path_metadata[PATH_MAX];

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  sprintfDatasetMd(path_metadata, d);
  struct stat statbuf;
  ret = stat(path_metadata, &statbuf);
  if (ret != 0) return ESDM_ERROR;
  off_t len = statbuf.st_size + 1;

  int fd = open(path_metadata, O_RDONLY);
  if (fd < 0) return ESDM_ERROR;
  char * json = ea_checked_malloc(len);
  ret = ea_read_check(fd, json, statbuf.st_size);
  close(fd);
  json[statbuf.st_size] = 0;
  if (ret != 0){
    return ESDM_ERROR;
  }
  *out_json = json;
  *out_size = statbuf.st_size;

  return ESDM_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int metadummy_backend_performance_estimate(esdm_md_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;
  *out_time = 0;

  return 0;
}

static int metadummy_finalize(esdm_md_backend_t *me) {
  DEBUG_ENTER;

  free(me->data);
  free(me->config);
  free(me);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_md_backend_t backend_template = {
  .name = "metadummy",
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    // General for ESDM
    .finalize = metadummy_finalize,                     // finalize
    .performance_estimate = metadummy_backend_performance_estimate, // performance_estimate

    .container_create = container_create,
    .container_commit = container_commit,
    .container_retrieve = container_retrieve,
    .container_remove = container_remove,

    .dataset_create = dataset_create,
    .dataset_commit = dataset_commit,
    .dataset_retrieve = dataset_retrieve,
    .dataset_remove = dataset_remove,

    .mkfs = mkfs,
    .fsck = fsck,
  },
};

esdm_md_backend_t *metadummy_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  esdm_md_backend_t *backend = ea_checked_malloc(sizeof(esdm_md_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_md_backend_t));

  metadummy_backend_options_t *data = ea_checked_malloc(sizeof(metadummy_backend_options_t));

  data->target = config->target;
  backend->data = data;
  backend->config = config;
  //metadummy_test();

  mkfs(backend, 0);

  return backend;
}
