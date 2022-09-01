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
#ifndef ESDM_BACKENDS_POSIXI_H
#define ESDM_BACKENDS_POSIXI_H

#include <esdm-internal.h>

#include <backends-data/generic-perf-model/lat-thr.h>

// Internal functions used by this backend.
typedef struct {
  esdm_config_backend_t *config;
  esdm_perf_model_lat_thp_t perf_model;
} posixi_backend_data_t;

// static int mkfs(esdm_backend_t* backend, int enforce_format);


/**
* Finalize callback implementation called on ESDM shutdown.
*
* This is the last chance for a backend to make outstanding changes persistent.
* This routine is also expected to clean up memory that is used by the backend.
*/

int posixi_finalize(esdm_backend_t *backend);

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

esdm_backend_t *posixi_backend_init(esdm_config_backend_t *config);

#endif
