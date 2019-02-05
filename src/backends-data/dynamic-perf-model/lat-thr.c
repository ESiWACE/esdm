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
#include <esdm-internal.h>
#include <backends-data/dynamic-perf-model/lat-thr.h>

void *esdm_backend_update_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data)
{
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

		// Write the first test data
		if ((*data->esdm_backend_check_dynamic_perf_model_lat_thp)(data->backend, data->size, &time1))
			continue;

		// Write the second test data
		if ((*data->esdm_backend_check_dynamic_perf_model_lat_thp)(data->backend, 2 * data->size, &time2))
			continue;

		// Sanity check
		time2 -= time1;	// Delta t
		if (time2 <= 0)
			continue;

		estimated_throughput = data->size / time2;
		estimated_latency = time1 - time2; // Optimized for data->size and 2 * data->size

		// Sanity check
		if ((estimated_throughput <= 0) || (estimated_latency < 0))
			continue;

		pthread_mutex_lock(&data->flag);
		data->throughput = data->alpha * data->throughput + (1.0 - data->alpha) * estimated_throughput;
		data->latency = data->alpha * data->latency + (1.0 - data->alpha) * estimated_latency;
		pthread_mutex_unlock(&data->flag);
	}

	return 0;
}

int esdm_backend_init_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data) {

	if (!data)
		return -1;

	pthread_mutex_init(&data->flag, NULL);

	return 0;
}

int esdm_backend_finalize_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data) {

	if (!data)
		return -1;

	if (data->tid)
		pthread_cancel(data->tid);
	pthread_mutex_destroy(&data->flag);

	if (data->log) {
		FILE *file = fopen(data->log, "a");
		if (file) {
			fprintf(file, "{ \"latency\": %f, \"throughput\" : %f }\n", data->latency, data->throughput);
			fclose(file);
		}
	}

	return 0;
}

int esdm_backend_parse_dynamic_perf_model_lat_thp(json_t * str, esdm_dynamic_perf_model_lat_thp_t * data)
{
	if (!str || !data)
		return -1;

	json_t *elem = NULL;
	elem = json_object_get(str, "latency");
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

	elem = json_object_get(str, "period");
	if (elem != NULL) {
		data->period = json_real_value(elem);
		assert(data->period > 0.0);
	} else
		data->period = 0.0;

	elem = json_object_get(str, "size");
	if (elem != NULL) {
		data->size = json_integer_value(elem);
		assert(data->size > 0);
	} else
		data->size = 0;

	elem = json_object_get(str, "alpha");
	if (elem != NULL) {
		data->alpha = json_real_value(elem);
		assert(data->alpha >= 0.0);
		assert(data->alpha < 1.0);
	} else
		data->alpha = 0.0;

	elem = json_object_get(str, "log");
	if (elem != NULL)
		data->log = json_string_value(elem);
	else
		data->log = NULL;

	data->tid = 0;
	data->backend = NULL;
	data->esdm_backend_check_dynamic_perf_model_lat_thp = NULL;

	return 0;
}

int esdm_backend_start_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data, esdm_backend * backend, int (*checker)(esdm_backend *, int, float *))
{
	if (!data)
		return -1;

	if (data->period > 0) {
		data->backend = backend;
		data->esdm_backend_check_dynamic_perf_model_lat_thp = checker;
		pthread_create(&data->tid, NULL, (void *(*)(void *)) &esdm_backend_update_dynamic_perf_model_lat_thp, data);
	}

	return 0;
}

int esdm_backend_reset_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data)
{
	if (!data)
		return -1;

	data->throughput = 0.0;
	data->latency = 0.0;
	data->period = 0.0;
	data->size = 0;
	data->alpha = 0.0;
	data->log = NULL;
	data->tid = 0;
	data->backend = NULL;
	data->esdm_backend_check_dynamic_perf_model_lat_thp = NULL;

	return 0;
}

int esdm_backend_estimate_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t* data, esdm_fragment_t *fragment, float * out_time)
{
	if (!data || !fragment | !out_time)
		return -1;

	pthread_mutex_lock(&data->flag);
	if (data->throughput > 0)
		*out_time = fragment->bytes / data->throughput + data->latency;
	else
		*out_time = 0.0;
	pthread_mutex_unlock(&data->flag);

	return 0;
}

