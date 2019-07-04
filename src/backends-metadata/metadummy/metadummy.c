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

#include <assert.h>
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

#include "metadummy.h"
#include <esdm-internal.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("METADUMMY", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("METADUMMY", fmt, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mkfs(esdm_md_backend_t *backend, int enforce_format) {
  DEBUG_ENTER;

  struct stat sb;

  // use target directory from backend configuration
  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;

  // Enforce min target length?
  const char *tgt = options->target;
  if (strlen(tgt) < 6) {
    printf("[mkfs] error, the target name is to short (< 6 characters)!\n");
    return ESDM_ERROR;
  }

  char containers[PATH_MAX];
  sprintf(containers, "%s/containers", tgt);
  if (enforce_format) {
    printf("[mkfs] Removing %s\n", tgt);
    if (stat(containers, &sb) != 0) {
      printf("[mkfs] error, this directory seems not to be created by ESDM!\n");
    } else {
      posix_recursive_remove(tgt);
    }
    if (enforce_format == 2) return ESDM_SUCCESS;
  }

  if (stat(tgt, &sb) == 0) {
    return ESDM_ERROR;
  }
  printf("[mkfs] Creating %s\n", tgt);

  int ret = mkdir(tgt, 0700);
  if (ret != 0) return ESDM_ERROR;

  ret = mkdir(containers, 0700);
  if (ret != 0) return ESDM_ERROR;

  return ESDM_SUCCESS;
}

static int fsck() {
  DEBUG_ENTER;

  return 0;
}

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
  if (fd != -1) {
    if ( json != NULL) {
      int ret = write_check(fd, json, size);
      assert(ret == 0);
    }
    close(fd);
    return 0;
  }

  return 1;
}

static int entry_retrieve_tst(const char *path, esdm_dataset_t *dataset) {
  DEBUG_ENTER;

  int status;
  struct stat sb;
  char *buf;

  DEBUG("entry_retrieve_tst(%s)\n", path);

  status = stat(path, &sb);
  if (status == -1) {
    perror("stat");
    // does not exist
    return -1;
  }

  //print_stat(sb);

  // write to non existing file
  int fd = open(path, O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

  // everything ok? write and close
  if (fd != -1) {
    // write some metadata
    buf = (char *)malloc(sb.st_size + 1);
    buf[sb.st_size] = 0;

    read_check(fd, buf, sb.st_size);
    close(fd);
  }

  DEBUG("Entry content: %s\n", (char *)buf);
  return 0;
}

static int entry_update(const char *path, void *buf, size_t len) {
  DEBUG_ENTER;

  int status;
  struct stat sb;

  DEBUG("entry_update(%s)\n", path);

  status = stat(path, &sb);
  if (status == -1) {
    perror("stat");
    return -1;
  }

  //print_stat(sb);

  // write to non existing file
  int fd = open(path, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

  // everything ok? write and close
  if (fd != -1) {
    // write some metadata
    write_check(fd, buf, len);
    close(fd);
  }

  return 0;
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

///////////////////////////////////////////////////////////////////////////////
// Container Helpers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int container_create(esdm_md_backend_t *backend, esdm_container_t *container) {
  DEBUG_ENTER;

  char *path_metadata;
  char *path_container;
  struct stat sb;

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  DEBUG("tgt: %p\n", tgt);

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  // create directory for datsets
  if (stat(path_container, &sb) == -1) {
    int ret = mkdir(path_container, 0700);
    if (ret != 0) return ESDM_ERROR;
  }

  // create metadata entry
  entry_create(path_metadata, NULL, 0);

  free(path_metadata);
  free(path_container);

  return 0;
}

static int container_retrieve(esdm_md_backend_t *backend, esdm_container_t *container) {
  DEBUG_ENTER;

  char *path_metadata;
  char *path_container;

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);


  free(path_metadata);
  free(path_container);

  return 0;
}

static int container_update(esdm_md_backend_t *backend, esdm_container_t *container) {
  DEBUG_ENTER;

  char *path_metadata;
  char *path_container;

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  // create metadata entry
  entry_update(path_metadata, "abc", 3);

  free(path_metadata);
  free(path_container);

  return 0;
}

static int container_destroy(esdm_md_backend_t *backend, esdm_container_t *container) {
  DEBUG_ENTER;

  char *path_metadata;
  char *path_container;

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  // create metadata entry
  entry_destroy(path_metadata);

  // TODO: also remove existing datasets?

  free(path_metadata);
  free(path_container);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Dataset Helpers ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int dataset_commit(esdm_md_backend_t *backend, esdm_dataset_t *dataset, char * json, int md_size) {
  DEBUG_ENTER;

  char path_metadata[PATH_MAX];
  char path_dataset[PATH_MAX];
  struct stat sb;

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  DEBUG("tgt: %p\n", tgt);

  sprintf(path_metadata, "%s/containers/%s/%s.md", tgt, dataset->container->name, dataset->name);
  sprintf(path_dataset, "%s/containers/%s/%s", tgt, dataset->container->name, dataset->name);
  // create directory for datsets
  if (stat(path_dataset, &sb) == -1) {
    int ret = mkdir_recursive(path_dataset);
    if (ret != 0) return ESDM_ERROR;
  }

  // create metadata entry
  entry_create(path_metadata, json, md_size);

  return 0;
}

static int dataset_retrieve(esdm_md_backend_t *backend, esdm_dataset_t *d, char ** out_json, int * out_size) {
  DEBUG_ENTER;
  int ret;
  char path_metadata[PATH_MAX];

  metadummy_backend_options_t *options = (metadummy_backend_options_t *)backend->data;
  const char *tgt = options->target;

  sprintf(path_metadata, "%s/containers/%s/%s.md", tgt, d->container->name, d->name);
  struct stat statbuf;
  ret = stat(path_metadata, &statbuf);
  if (ret != 0) return ESDM_ERROR;
  off_t len = statbuf.st_size + 1;

  int fd = open(path_metadata, O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
  if (fd < 0) return ESDM_ERROR;
  char * json = (char *)malloc(len);
  ret = read_check(fd, json, statbuf.st_size);
  close(fd);
  json[statbuf.st_size] = 0;
  if (ret != 0){
    return ESDM_ERROR;
  }
  *out_json = json;
  *out_size = statbuf.st_size;

  return ESDM_SUCCESS;
}

static int dataset_update(esdm_md_backend_t *backend, esdm_dataset_t *dataset) {
  DEBUG_ENTER;

  return 0;
}

static int dataset_destroy(esdm_md_backend_t *backend, esdm_dataset_t *dataset) {
  DEBUG_ENTER;

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int metadummy_backend_performance_estimate(esdm_md_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;
  *out_time = 0;

  return 0;
}

static int metadummy_finalize(esdm_md_backend_t *b) {
  DEBUG_ENTER;

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
metadummy_finalize,                     // finalize
metadummy_backend_performance_estimate, // performance_estimate

container_create,
container_retrieve,
container_update,
container_destroy,

dataset_commit,
dataset_retrieve,
dataset_update,
dataset_destroy,

mkfs,
},
};

esdm_md_backend_t *metadummy_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  esdm_md_backend_t *backend = (esdm_md_backend_t *)malloc(sizeof(esdm_md_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_md_backend_t));

  metadummy_backend_options_t *data = (metadummy_backend_options_t *)malloc(sizeof(metadummy_backend_options_t));

  data->target = config->target;
  backend->data = data;
  backend->config = config;
  //metadummy_test();

  mkfs(backend, 0);

  return backend;
}
