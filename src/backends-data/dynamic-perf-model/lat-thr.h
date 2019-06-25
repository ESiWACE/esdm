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
#ifndef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_H
#define ESDM_BACKENDS_DYNAMIC_PERF_MODEL_H

#include <jansson.h>
#include <pthread.h>

#include <esdm.h>

typedef struct {
	double latency;	// seconds
	double throughput;	// bytes per seconds
	int size;	// bytes for test
	esdm_backend * backend;	// reference to the corresponding backend
	int (*esdm_backend_check_dynamic_perf_model_lat_thp)(esdm_backend *, int, float *);	// Function to write data and remove
#ifdef ESDM_BACKENDS_DYNAMIC_PERF_MODEL_WITH_THREAD
	double period;	// seconds
	double alpha;
	pthread_t tid;
	pthread_mutex_t flag;
#endif
} esdm_dynamic_perf_model_lat_thp_t;

int esdm_backend_init_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data);
int esdm_backend_finalize_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data);

int esdm_backend_parse_dynamic_perf_model_lat_thp(json_t * perf_model_str, esdm_dynamic_perf_model_lat_thp_t * out_data);

int esdm_backend_start_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data, esdm_backend * backend, int (*checker)(esdm_backend *, int, float *));

int esdm_backend_reset_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data);

int esdm_backend_update_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t* data);

int esdm_backend_estimate_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t * data, esdm_fragment_t * fragment, float *out_time);

int esdm_backend_get_dynamic_perf_model_lat_thp(esdm_dynamic_perf_model_lat_thp_t* data, char **json);

#endif
