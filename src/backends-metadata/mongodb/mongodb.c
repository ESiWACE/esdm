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

 
#include <bson.h>
#include <esdm.h>
#include <fcntl.h>
#include <jansson.h>
#include <mongoc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mongodb.h"

#define DEBUG(msg) log("[METADUMMY] %-30s %s:%d\n", msg, __FILE__, __LINE__)

static void log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mkfs(esdm_backend_t *backend) {
  DEBUG("mongodb setup");

  struct stat sb;

  // use target directory from backend configuration
  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;
  //const char* tgt = "./_mongodb";

  if (stat(tgt, &sb) == -1) {
    char *root;
    char *containers;

    asprintf(&root, "%s", tgt);
    mkdir(root, 0700);

    asprintf(&containers, "%s/containers", tgt);
    mkdir(containers, 0700);

    free(root);
    free(containers);
  }
}

static int fsck() {
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Internal Helpers  //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_create(const char *path) {
  int status;
  struct stat sb;

  printf("entry_create(%s)\n", path);

  // ENOENT => allow to create

  status = stat(path, &sb);
  if (status == -1) {
    perror("stat");

    // write to non existing file
    int fd = open(path, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

    // everything ok? write and close
    if (fd != -1) {
      close(fd);
    }

    return 0;

  } else {
    // already exists
    return -1;
  }
}

static int entry_retrieve(const char *path) {
  int status;
  struct stat sb;
  char *buf;

  printf("entry_retrieve(%s)\n", path);

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
    buf = ea_checked_malloc(sb.st_size + 1);
    buf[sb.st_size] = 0;

    read(fd, buf, sb.st_size);
    close(fd);
  }

  printf("Entry content: %s\n", (char *)buf);

  return 0;
}

static int entry_update(const char *path, void *buf, size_t len) {
  int status;
  struct stat sb;

  printf("entry_update(%s)\n", path);

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
    write(fd, buf, len);
    close(fd);
  }

  return 0;
}

static int entry_destroy(const char *path) {
  int status;
  struct stat sb;

  printf("entry_destroy(%s)\n", path);

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

static int container_create(esdm_backend_t *backend, esdm_container_t *container) {
  char *path_metadata;
  char *path_container;
  struct stat sb;

  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;

  printf("tgt: %p\n", tgt);

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  bson_error_t error;
  bson_oid_t oid;
  bson_t *doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);
  BSON_APPEND_UTF8(doc, "container", container->name);

  if (!mongoc_collection_insert(options->collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
    printf("%s\n", error.message);
  }

  bson_destroy(doc);

  // create metadata entry
  entry_create(path_metadata);

  // create directory for datsets
  if (stat(path_container, &sb) == -1) {
    mkdir(path_container, 0700);
  }

  free(path_metadata);
  free(path_container);
}

static int container_retrieve(esdm_backend_t *backend, esdm_container_t *container) {
  char *path_metadata;
  char *path_container;
  struct stat sb;

  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  // create metadata entry
  entry_retrieve(path_metadata);

  free(path_metadata);
  free(path_container);
}

static int container_update(esdm_backend_t *backend, esdm_container_t *container) {
  char *path_metadata;
  char *path_container;
  struct stat sb;

  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  // create metadata entry
  entry_update(path_metadata, "abc", 3);

  free(path_metadata);
  free(path_container);
}

static int container_destroy(esdm_backend_t *backend, esdm_container_t *container) {
  char *path_metadata;
  char *path_container;
  struct stat sb;

  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;

  asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
  asprintf(&path_container, "%s/containers/%s", tgt, container->name);

  // create metadata entry
  entry_destroy(path_metadata);

  // TODO: also remove existing datasets?

  free(path_metadata);
  free(path_container);
}

///////////////////////////////////////////////////////////////////////////////
// Dataset Helpers ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int dataset_create(esdm_backend_t *backend, esdm_dataset_t *dataset) {
  char *path_metadata;
  char *path_dataset;
  struct stat sb;

  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;

  printf("tgt: %p\n", tgt);

  asprintf(&path_metadata, "%s/containers/%s/%s.md", tgt, dataset->container->name, dataset->name);
  asprintf(&path_dataset, "%s/containers/%s/%s", tgt, dataset->container->name, dataset->name);

  bson_error_t error;
  bson_oid_t oid;
  bson_t *doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);
  BSON_APPEND_UTF8(doc, "dataset", dataset->name);

  if (!mongoc_collection_insert(options->collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
    printf("%s\n", error.message);
  }

  bson_destroy(doc);

  // create metadata entry
  entry_create(path_metadata);

  // create directory for datsets
  if (stat(path_dataset, &sb) == -1) {
    mkdir(path_dataset, 0700);
  }

  free(path_metadata);
  free(path_dataset);
}

static int dataset_retrieve(esdm_backend_t *backend, esdm_dataset_t *dataset) {
}

static int dataset_update(esdm_backend_t *backend, esdm_dataset_t *dataset) {
}

static int dataset_destroy(esdm_backend_t *backend, esdm_dataset_t *dataset) {
}

///////////////////////////////////////////////////////////////////////////////
// Fragment Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//TODO this has zero test coverage currently
static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *fragment) {
  char *path;
  char *path_fragment;
  struct stat sb;

  mongodb_backend_options_t *options = (mongodb_backend_options_t *)backend->data;
  const char *tgt = options->target;

  printf("tgt: %p\n", tgt);

  asprintf(&path, "%s/containers/%s/%s/", tgt, fragment->dataset->container->name, fragment->dataset->name);
  asprintf(&path_fragment, "%s/containers/%s/%s/%p", tgt, fragment->dataset->container->name, fragment->dataset->name, fragment);

  printf("path: %s\n", path);
  printf("path_fragment: %s\n", path_fragment);

  // create metadata entry
  //mkdir_recursive(path);
  //entry_create(path_fragment);

  //	entry_update(path_fragment, "abc", 3);

  //entry_update()

  /*
  size_t *count = NULL;
  void *buf = NULL;
  entry_retrieve(path_fragment, &buf, &count);
  */

  free(path);
  free(path_fragment);
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mongodb_backend_performance_estimate(esdm_backend_t *backend) {
  DEBUG("Calculating performance estimate.");

  return 0;
}

static int mongodb_create(esdm_backend_t *backend, char *name) {
  DEBUG("Create");

  // TODO; Sanitize name, and reject forbidden names

  //container_create(backend, name);

  //#include <unistd.h>
  //ssize_t pread(int fd, void *buf, size_t count, off_t offset);
  //ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

  return 0;
}

/**
 *
 *	handle
 *	mode
 *	owner?
 *
 */
static int mongodb_open(esdm_backend_t *backend) {
  DEBUG("Open");
  return 0;
}

static int mongodb_write(esdm_backend_t *backend) {
  DEBUG("Write");
  return 0;
}

static int mongodb_read(esdm_backend_t *backend) {
  DEBUG("Read");
  return 0;
}

static int mongodb_close(esdm_backend_t *backend) {
  DEBUG("Close");
  return 0;
}

static int mongodb_allocate(esdm_backend_t *backend) {
  DEBUG("Allocate");
  return 0;
}

static int mongodb_update(esdm_backend_t *backend) {
  DEBUG("Update");
  return 0;
}

static int mongodb_lookup(esdm_backend_t *backend) {
  DEBUG("Lookup");
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
///////////////////////////////////////////////////////////////////////////////
// WARNING: This serves as a template for the mongodb plugin and is memcpied!  //
///////////////////////////////////////////////////////////////////////////////
.name = "mongodb",
.type = ESDM_MODULE_METADATA,
.version = "0.0.1",
.data = NULL,
.callbacks = {
// General for ESDM
NULL,                                 // finalize, FIXME: We leak memory because this is not implemented.
mongodb_backend_performance_estimate, // performance_estimate

// Data Callbacks (POSIX like)
NULL, // create
NULL, // open
NULL, // write
NULL, // read
NULL, // close

// Metadata Callbacks
NULL, // lookup

// ESDM Data Model Specific
container_create,   // container create
container_retrieve, // container retrieve
container_update,   // container update
container_destroy,  // container destroy

dataset_create,   // dataset create
dataset_retrieve, // dataset retrieve
dataset_update,   // dataset update
dataset_destroy,  // dataset destroy

NULL,            // fragment create
NULL,            // fragment retrieve
fragment_update, // fragment update
NULL,            // fragment destroy
},
};

esdm_backend_t *mongodb_backend_init(esdm_config_backend_t *config) {
  DEBUG("Initializing mongodb backend.");

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  mongodb_backend_options_t *data = ea_checked_malloc(sizeof(mongodb_backend_options_t));

  char *tgt;
  asprintf(&tgt, "./_mongodb");
  data->target = tgt;

  //backend->data = init_data;
  backend->data = data;

  // todo check mongodb style persitency structure available?
  mkfs(backend);

  mongoc_client_t *client;
  mongoc_collection_t *collection;
  mongoc_database_t *database;

  //Required to initialize libmongoc's internals
  mongoc_init();

  // connect to database and select collection
  client = mongoc_client_new("mongodb://localhost:27017");

  // TODO: How should this plugin act in a multiprocessor environemnt? Do not make multiple connections per node
  mongoc_client_set_appname(client, "ESDM Backend");
  collection = mongoc_client_get_collection(client, "esdm", "esdm");

  data->client = client;
  data->collection = collection;

  //mongodb_test();
  //mongoconnect(0, NULL);

  free(config); //We do not use it here, so we just get rid of it.

  return backend;
}

int mongodb_finalize() {
  //mongoc_collection_destroy (collection);
  //mongoc_client_destroy (client);

  return 0;
}

static void mongodb_test() {
  int ret = -1;

  char *abc;
  char *def;

  const char *tgt = "./_mongodb";
  asprintf(&abc, "%s/%s", tgt, "abc");
  asprintf(&def, "%s/%s", tgt, "def");

  // create entry and test
  ret = entry_create(abc);
  eassert(ret == 0);

  ret = entry_retrieve(abc);
  eassert(ret == 0);

  // double create
  ret = entry_create(def);
  eassert(ret == 0);

  ret = entry_create(def);
  eassert(ret == -1);

  // perform update and test
  ret = entry_update(abc, "huhuhuhuh", 5);
  ret = entry_retrieve(abc);

  // delete entry and expect retrieve to fail
  ret = entry_destroy(abc);
  ret = entry_retrieve(abc);
  eassert(ret == -1);

  // clean up
  ret = entry_destroy(def);
  eassert(ret == 0);

  ret = entry_destroy(def);
  eassert(ret == -1);
}
