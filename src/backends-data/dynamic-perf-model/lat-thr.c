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
#include <backends-data/dynamic-perf-model/lat-thr.h>
#include <esdm-internal.h>

#define ESDM_BACKENDS_DYNAMIC_PERF_MODEL_SIZE 256

int _esdm_backend_t_update_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data, double *estimated_throughput, double *estimated_latency) {
  if (!data || !estimated_throughput || !estimated_latency)
    return 1;

  float time1, time2;

  // Write the first test data
  if ((*data->esdm_backend_t_check_dynamic_perf_model_lat_thp)(data->backend, data->size, &time1))
    return 2;

  // Write the second test data
  if ((*data->esdm_backend_t_check_dynamic_perf_model_lat_thp)(data->backend, 2 * data->size, &time2))
    return 3;

  // Sanity check
  time2 -= time1; // Delta t
  if (time2 <= 0)
    return 4;

  *estimated_throughput = data->size / time2;
  *estimated_latency    = time1 - time2; // Optimized for data->size and 2 * data->size

  // Sanity check
  if ((*estimated_throughput <= 0) || (*estimated_latency < 0))
    return 5;

  return 0;
}

#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD

void *esdm_backend_t_autoupdate_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data) {
  pthread_detach(pthread_self());

  if (!data)
    return 0;

  char first = 1;
  float time1, time2;
  double estimated_throughput, estimated_latency;

  while ((data->period > 0) && (data->size > 0) && data->backend) {
    if (first)
      first = 0;
    else
      sleep(data->period);

    if (_esdm_backend_t_update_dynamic_perf_model_lat_thp(data, &estimated_throughput, &estimated_latency))
      continue;

    pthread_mutex_lock(&data->flag);
    data->throughput = data->alpha * data->throughput + (1.0 - data->alpha) * estimated_throughput;
    data->latency    = data->alpha * data->latency + (1.0 - data->alpha) * estimated_latency;
    pthread_mutex_unlock(&data->flag);
  }

  return 0;
}

#endif

int esdm_backend_t_init_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data) {
  if (!data)
    return -1;

#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD

  pthread_mutex_init(&data->flag, NULL);

#endif

  return 0;
}

int esdm_backend_t_finalize_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data) {
  if (!data)
    return -1;

#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD

  if (data->tid)
    pthread_cancel(data->tid);
  pthread_mutex_destroy(&data->flag);

#endif

  return 0;
}

int esdm_backend_t_parse_dynamic_perf_model_lat_thp(json_t *str, esdm_dynamic_perf_model_lat_thp_t *data) {
  if (!str || !data)
    return -1;

  json_t *elem = NULL;
  elem         = json_object_get(str, "latency");
  if (elem != NULL) {
    data->latency = json_real_value(elem);
    assert(data->latency >= 0.0);
  } else
    data->latency = 0.0;

  elem = json_object_get(str, "throughput");
  if (elem != NULL) {
    data->throughput = json_real_value(elem) * 1024 * 1024;
    assert(data->throughput > 0.0);
  } else
    data->throughput = 0.0;

  elem = json_object_get(str, "size");
  if (elem != NULL) {
    data->size = json_integer_value(elem);
    assert(data->size > 0);
  } else
    data->size = 0;

#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD

  elem = json_object_get(str, "period");
  if (elem != NULL) {
    data->period = json_real_value(elem);
    assert(data->period > 0.0);
  } else
    data->period = 0.0;

  elem = json_object_get(str, "alpha");
  if (elem != NULL) {
    data->alpha = json_real_value(elem);
    assert(data->alpha >= 0.0);
    assert(data->alpha < 1.0);
  } else
    data->alpha = 0.0;

  data->tid = 0;

#endif

  data->backend                                         = NULL;
  data->esdm_backend_t_check_dynamic_perf_model_lat_thp = NULL;

  return 0;
}

int esdm_backend_t_start_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data, esdm_backend_t *backend, int (*checker)(esdm_backend_t *, int, float *)) {
  if (!data)
    return -1;

  data->backend                                         = backend;
  data->esdm_backend_t_check_dynamic_perf_model_lat_thp = checker;

#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD

  pthread_create(&data->tid, NULL, (void *(*)(void *)) & esdm_backend_t_autoupdate_dynamic_perf_model_lat_thp, data);

#else

  // Estimate initial performance
  esdm_backend_t_update_dynamic_perf_model_lat_thp(data);

#endif

  return 0;
}

int esdm_backend_t_reset_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data) {
  if (!data)
    return -1;

  data->throughput                                      = 0.0;
  data->latency                                         = 0.0;
  data->size                                            = 0;
  data->backend                                         = NULL;
  data->esdm_backend_t_check_dynamic_perf_model_lat_thp = NULL;
#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD
  data->period = 0.0;
  data->alpha  = 0.0;
  data->tid    = 0;
#endif

  return 0;
}

int esdm_backend_t_update_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data) {
  if (!data)
    return -1;

  double estimated_throughput, estimated_latency;
  if (!_esdm_backend_t_update_dynamic_perf_model_lat_thp(data, &estimated_throughput, &estimated_latency)) {
    data->throughput = estimated_throughput;
    data->latency    = estimated_latency;
  }

  return 0;
}

int esdm_backend_t_estimate_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data, esdm_fragment_t *fragment, float *out_time) {
  if (!data || !fragment | !out_time)
    return -1;

#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD

  pthread_mutex_lock(&data->flag);
  if (data->throughput > 0)
    *out_time = fragment->bytes / data->throughput + data->latency;
  else
    *out_time = 0.0;
  pthread_mutex_unlock(&data->flag);

#else

  if (data->throughput > 0) {
    /*
		// Update current performance estimate
		esdm_backend_t_update_dynamic_perf_model_lat_thp(data);
*/
    *out_time = fragment->bytes / data->throughput + data->latency;

  } else
    *out_time = 0.0;

#endif

  return 0;
}

int esdm_backend_t_get_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t *data, char **json) {
  if (!data || !json)
    return -1;

  char tmp[ESDM_BACKENDS_DYNAMIC_PERF_MODEL_SIZE];
  snprintf(tmp, ESDM_BACKENDS_DYNAMIC_PERF_MODEL_SIZE, "{ \"latency\": %f, \"throughput\" : %f }", data->latency, data->throughput / 1024 / 1024);

  *json = strdup(tmp);
  if (!*json)
    return -2;

  return 0;
}
