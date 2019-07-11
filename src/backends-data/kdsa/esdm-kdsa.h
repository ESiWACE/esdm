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
} kdsa_backend_data_t;


int posix_finalize(esdm_backend_t *backend);

esdm_backend_t *kdsa_backend_init(esdm_config_backend_t *config);

#endif
