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
#ifndef ESDM_BACKENDS_POSIX_H
#define ESDM_BACKENDS_POSIX_H

#include <esdm-internal.h>

#include <backends-data/generic-perf-model/lat-thr.h>

// Internal functions used by this backend.
typedef struct {
	esdm_config_backend_t *config;
	const char *target;
	esdm_perf_model_lat_thp_t perf_model;
} posix_backend_data_t;


// static int mkfs(esdm_backend_t* backend, int enforce_format);

/**
 * Similar to the command line counterpart fsck for ESDM plugins is responsible
 * to check and potentially repair the "filesystem".
 *
 */

// static int fsck();
//
// static int entry_create(const char *path);
//
// static int entry_retrieve(const char *path, void *buf);
//
// static int entry_update(const char *path, void *buf, size_t len);
//
// static int entry_destroy(const char *path);
//
// static int fragment_retrieve(esdm_backend_t* backend, esdm_fragment_t *fragment, json_t * metadata);
//
// static int fragment_update(esdm_backend_t* backend, esdm_fragment_t *fragment);
//
// /**
//  * Callback implementation when beeing queried for a performance estimate.
//  *
//  */
//
// static int posix_backend_performance_estimate(esdm_backend_t* backend, esdm_fragment_t *fragment, float * out_time);

/**
* Finalize callback implementation called on ESDM shutdown.
*
* This is the last chance for a backend to make outstanding changes persistent.
* This routine is also expected to clean up memory that is used by the backend.
*/

int posix_finalize(esdm_backend_t* backend);

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


esdm_backend_t* posix_backend_init(esdm_config_backend_t *config);


#endif
