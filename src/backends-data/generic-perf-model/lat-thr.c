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
#include <esdm-debug.h>
#include <backends-data/generic-perf-model/lat-thr.h>

int esdm_backend_parse_perf_model_lat_thp(json_t * str, esdm_perf_model_lat_thp_t * out_data){
  json_t *elem = NULL;
  elem = json_object_get(str, "latency");
  assert(elem != NULL);
  out_data->latency_in_s = json_real_value(elem);
  elem = json_object_get(str, "throughput");
  assert(elem != NULL);
  out_data->throughputBs = json_real_value(elem) * 1024 * 1024;

  assert(out_data->throughputBs > 0);

  return 0;
}

int esdm_backend_perf_model_long_lat_perf_estimate(esdm_perf_model_lat_thp_t* data, esdm_fragment_t *fragment, float * out_time){
  *out_time = fragment->size / data->throughputBs + data->latency_in_s;
  return 0;
}
