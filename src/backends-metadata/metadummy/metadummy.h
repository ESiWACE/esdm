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
#ifndef METADUMMY_H
#define METADUMMY_H

#include <esdm-internal.h>

// Internal functions used by this backend.
typedef struct {
  const char *type;
  const char *name;
  const char *target;
} metadummy_backend_options_t;


// Internal functions used by this backend.
typedef struct {
  metadummy_backend_options_t *options;
  int other;
} metadummy_backend_data_t;


typedef struct md_entry_struct md_entry_t;

struct md_entry_struct {
  int type;
};


// forward declarations

// static void metadummy_test();

void posix_recursive_remove(const char *path);

// static int mkfs(esdm_md_backend_t* backend, int enforce_format);

/**
 * Similar to the command line counterpart fsck for ESDM plugins is responsible
 * to check and potentially repair the "filesystem".
 *
 */

// static int fsck();
//
// static int entry_create(const char *path, esdm_metadata_t *data);
//
// static int entry_retrieve_tst(const char *path, esdm_dataset_t *dataset);
//
// static int entry_update(const char *path, void *buf, size_t len);
//
// static int entry_destroy(const char *path);
//
// static int container_create(esdm_md_backend_t* backend, esdm_container_t *container);
//
// static int container_retrieve(esdm_md_backend_t* backend, esdm_container_t *container);
//
// static int container_update(esdm_md_backend_t* backend, esdm_container_t *container);
//
// static int container_destroy(esdm_md_backend_t* backend, esdm_container_t *container);
//
// static int dataset_create(esdm_md_backend_t* backend, esdm_dataset_t *dataset);
//
// static int dataset_retrieve(esdm_md_backend_t* backend, esdm_dataset_t *dataset);
//
// static int dataset_update(esdm_md_backend_t* backend, esdm_dataset_t *dataset);
//
// static int fragment_retrieve(esdm_md_backend_t* backend, esdm_fragment_t *fragment, json_t * metadata);
//
// static esdm_fragment_t * create_fragment_from_metadata(int fd, esdm_dataset_t * dataset, esdm_dataspace_t * space);
//
// static int lookup(esdm_md_backend_t* backend, esdm_dataset_t * dataset, esdm_dataspace_t * space, int * out_frag_count, esdm_fragment_t *** out_fragments);
//
// static int fragment_update(esdm_md_backend_t* backend, esdm_fragment_t *fragment);
//
// static int metadummy_backend_performance_estimate(esdm_md_backend_t* backend, esdm_fragment_t *fragment, float * out_time);

/**
* Finalize callback implementation called on ESDM shutdown.
*
* This is the last chance for a backend to make outstanding changes persistent.
* This routine is also expected to clean up memory that is used by the backend.
*/

// static int metadummy_finalize(esdm_md_backend_t* b);

/**
* Initializes the POSIX plugin. In particular this involves:
*
*	* Load configuration of this backend
*	* Load and potentially calibrate performance model
*
*	* Connect with support services e.g. for technical metadata
*	* Setup directory structures used by this POSIX specific backend
*
*	* Populate esdm_md_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/

esdm_md_backend_t *metadummy_backend_init(esdm_config_backend_t *config);

// static void metadummy_test();

#endif
