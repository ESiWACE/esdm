/* This file is part of ESDM.
 *
 * This program is is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ESDM_BACKENDS_PERF_MODEL_H
#define ESDM_BACKENDS_PERF_MODEL_H

#include <jansson.h>

#include <esdm.h>

typedef struct{
  double latency_in_s;
  double throughputBs;
} esdm_perf_model_lat_thp_t;

int esdm_backend_parse_perf_model_lat_thp(json_t * perf_model_str, esdm_perf_model_lat_thp_t * out_data);

int esdm_backend_perf_model_long_lat_perf_estimate(esdm_perf_model_lat_thp_t* data, esdm_fragment_t *fragment, float * out_time);

#endif
