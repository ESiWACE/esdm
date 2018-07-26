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


/**
 * @file
 * @brief The scheduler receives application requests and schedules subsequent
 *        I/O requests as a are necessary for metadata lookups and data
 *        reconstructions.
 *
 */


#include <esdm.h>
#include <esdm-internal.h>

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("SCHEDULER", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("SCHEDULER", fmt, __VA_ARGS__)

static void backend_thread(io_work_t* data_p, esdm_backend_t* backend_id);

esdm_scheduler_t* esdm_scheduler_init(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);

	esdm_scheduler_t* scheduler = NULL;
	scheduler = (esdm_scheduler_t*) malloc(sizeof(esdm_scheduler_t));

  // create thread pools per device
  // decide how many threads should be used per backend.
  int ppn = esdm->procs_per_node;
  GError * error;
	for (int i = 0; i < esdm->modules->backend_count; i++) {
		esdm_backend_t* b = esdm->modules->backends[i];
    int max_threads = b->config->max_threads_per_node;
    int cur_threads = max_threads / ppn;
    if (cur_threads == 0){
      b->threadPool = NULL;
    }else{
      b->threadPool = g_thread_pool_new((GFunc)(backend_thread), b, cur_threads, 1, & error);
    }
  }

	return scheduler;
}

static void backend_thread(io_work_t* data, esdm_backend_t* backend_id){
  io_request_status_t * status = data->parent;
  g_mutex_lock(& status->mutex);
  status->pending_ops--;
  assert(status->pending_ops >= 0);
  if( status->pending_ops == 0){
    g_cond_signal(& status->done_condition);
  }
  g_mutex_unlock(& status->mutex);
  free(data);
}

esdm_status_t esdm_scheduler_finalize(esdm_instance_t *esdm)
{
  for (int i = 0; i < esdm->modules->backend_count; i++) {
    esdm_backend_t* b = esdm->modules->backends[i];
    if(b->threadPool){
      g_thread_pool_free(b->threadPool, 0, 1);
    }
  }
	ESDM_DEBUG(__func__);
	return ESDM_SUCCESS;
}

esdm_status_t esdm_scheduler_enqueue(esdm_instance_t *esdm, io_request_status_t * status, io_operation_t type, esdm_dataspace_t* subspace){
    GError * error;
    //Gather I/O recommendations
    //esdm_performance_recommendation(esdm, NULL, NULL);    // e.g., split, merge, replication?
    //esdm_layout_recommendation(esdm, NULL, NULL);		  // e.g., merge, split, transform?

    // now enqueue the operations
    return ESDM_SUCCESS;
}

esdm_status_t esdm_scheduler_status_init(io_request_status_t * status){
  g_mutex_init(& status->mutex);
  g_cond_init(& status->done_condition);
  status->pending_ops = 0;
  return ESDM_SUCCESS;
}

esdm_status_t esdm_scheduler_status_finalize(io_request_status_t * status){
  g_mutex_clear(& status->mutex);
  g_cond_clear(& status->done_condition);
  return ESDM_SUCCESS;
}

esdm_status_t esdm_scheduler_wait(io_request_status_t * status){
    g_mutex_lock(& status->mutex);
    if (status->pending_ops){
      g_cond_wait(& status->done_condition, & status->mutex);
    }
    g_mutex_unlock(& status->mutex);
    return ESDM_SUCCESS;
}

esdm_status_t esdm_scheduler_process_blocking(esdm_instance_t *esdm, io_operation_t type, esdm_dataspace_t* subspace){
	ESDM_DEBUG(__func__);

  io_request_status_t status;

	esdm_status_t ret;

  ret = esdm_scheduler_status_init(& status);
  assert( ret == ESDM_SUCCESS );
  ret = esdm_scheduler_enqueue(esdm, & status, type, subspace);
  assert( ret == ESDM_SUCCESS );

  ret = esdm_scheduler_wait(& status);
  assert( ret == ESDM_SUCCESS );
  ret = esdm_scheduler_status_finalize(& status);
  assert( ret == ESDM_SUCCESS );
	return ESDM_SUCCESS;
}





esdm_status_t esdm_backend_io(esdm_backend_t* backend, esdm_fragment_t* fragment, esdm_metadata_t* metadata)
{
	ESDM_DEBUG(__func__);

	return ESDM_SUCCESS;
}
