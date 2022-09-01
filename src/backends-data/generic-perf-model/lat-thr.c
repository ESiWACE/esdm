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
#include <backends-data/generic-perf-model/lat-thr.h>
#include <esdm-internal.h>

int esdm_backend_t_parse_perf_model_lat_thp(json_t *str, esdm_perf_model_lat_thp_t *out_data) {
  if (!str || !out_data)
    return -1;

  json_t *elem = NULL;
  elem = jansson_object_get(str, "latency");
  eassert(elem != NULL);
  out_data->latency_in_s = json_real_value(elem);

  elem = jansson_object_get(str, "throughput");
  eassert(elem != NULL);
  out_data->throughputBs = json_real_value(elem) * 1024 * 1024;

  eassert(out_data->latency_in_s >= 0);
  eassert(out_data->throughputBs > 0);

  return 0;
}

float esdm_backend_t_perf_model_get_throughput(esdm_perf_model_lat_thp_t* me) {
  eassert(me);
  return me->throughputBs > 0 ? me->throughputBs : 100.0*1024*1024; //default to 100 MiB/s if no throughput was set
}

int esdm_backend_t_perf_model_long_lat_perf_estimate(esdm_perf_model_lat_thp_t *data, esdm_fragment_t *fragment, float *out_time) {
  if (!data || !fragment | !out_time)
    return -1;

  if (data->throughputBs > 0)
    *out_time = fragment->bytes / data->throughputBs + data->latency_in_s;
  else
    *out_time = 0;

  return 0;
}

int esdm_backend_t_reset_perf_model_lat_thp(esdm_perf_model_lat_thp_t *out_data) {
  if (!out_data)
    return -1;

  out_data->throughputBs = 0;
  out_data->latency_in_s = 0;

  return 0;
}
