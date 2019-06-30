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


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <esdm-debug.h>

#include "metadummy.h"

#define DEBUG_ENTER(msg) ESDM_DEBUG_COM_FMT("DUMMY", "%s", msg)


///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


int mkfs(esdm_backend_t *backend) {
  posix_backend_data_t *data       = (posix_backend_data_t *)backend->data;
  posix_backend_options_t *options = data->options;

  ESDM_DEBUG("mkfs: backend->(void*)data->options->target = %s\n", options->target);
  ESDM_DEBUG("\n\n\n");


  const char *tgt = options->target;

  struct stat st = {0};

  if (stat(tgt, &st) == -1) {
    char *root;
    char *cont;
    char *sdat;
    char *sfra;

    asprintf(&root, "%s", tgt);
    asprintf(&cont, "%s/containers", tgt);
    asprintf(&sdat, "%s/shared-datasets", tgt);
    asprintf(&sfra, "%s/shared-fragments", tgt);

    mkdir(root, 0700);
    mkdir(cont, 0700);
    mkdir(sdat, 0700);
    mkdir(sfra, 0700);

    free(root);
    free(cont);
    free(sdat);
    free(sfra);
  }
}


/**
 * Similar to the command line counterpart fsck for ESDM plugins is responsible
 * to check and potentially repair the "filesystem".
 *
 */
int fsck() {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Internal Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// find_fragment

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int posix_backend_performance_estimate(esdm_backend_t *backend) {
  DEBUG_ENTER("Calculating performance estimate.");

  posix_backend_data_t *data       = (posix_backend_data_t *)backend->data;
  posix_backend_options_t *options = data->options;

  ESDM_DEBUG("perf_estimate: backend->(void*)data->options->target = %s\n", options->target);

  return 0;
}


int posix_create() {
  DEBUG_ENTER("Create");


  // check if container already exists

  struct stat st = {0};
  if (stat("_esdm-fs", &st) == -1) {
    mkdir("_esdm-fs/containers", 0700);
  }


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
int posix_open() {
  DEBUG_ENTER("Open");
  return 0;
}


int posix_write() {
  DEBUG_ENTER("Write");
  return 0;
}


int posix_read() {
  DEBUG_ENTER("Read");
  return 0;
}


int posix_close() {
  DEBUG_ENTER("Close");
  return 0;
}


int posix_allocate() {
  DEBUG_ENTER("Allocate");
  return 0;
}


int posix_update() {
  DEBUG_ENTER("Update");
  return 0;
}


int posix_lookup() {
  DEBUG_ENTER("Lookup");
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
///////////////////////////////////////////////////////////////////////////////
// WARNING: This serves as a template for the posix plugin and is memcpied!  //
///////////////////////////////////////////////////////////////////////////////
.name      = "metadummy",
.type      = SMD_DTYPE_METADATA,
.version   = "0.0.1",
.data      = NULL,
.callbacks = {
NULL,                               // finalize
posix_backend_performance_estimate, // performance_estimate

posix_create, // create
posix_open,   // open
posix_write,  // write
posix_read,   // read
posix_close,  // close

NULL, // allocate
NULL, // update
NULL, // lookup
},
};

/**
* Initializes the POSIX plugin. In particular this involves:
*
*	* Load configuration of this backend
*	* Load and potentially calibrate performance model
*
*	* Connect with support services e.g. for technical metadata
*	* Setup directory structures used by this POSIX specific backend
*
*	* Populate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/

// Two versions of this function!!!
//
// posix.c

esdm_backend_t *posix_backend_init(void *init_data) {
  DEBUG_ENTER("Initializing POSIX backend.");

  esdm_backend_t *backend = (esdm_backend_t *)malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  backend->data                    = (void *)malloc(sizeof(posix_backend_data_t));
  posix_backend_data_t *data       = (posix_backend_data_t *)backend->data;
  posix_backend_options_t *options = (posix_backend_options_t *)init_data;
  data->options                    = options;

  // valid refs for backend, data, options available now
  data->other = 47;

  // todo check posix style persitency structure available?
  mkfs(backend);

  return backend;
}

/**
* Initializes the POSIX plugin. In particular this involves:
*
*/
int posix_finalize() {
  return 0;
}
