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
#include <string.h>
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
  const int ppn = esdm->procs_per_node;
  const int gt = esdm->total_procs;
  GError * error;
	for (int i = 0; i < esdm->modules->backend_count; i++) {
		esdm_backend_t* b = esdm->modules->backends[i];
		// in total we should not use more than max_global total threads
		int max_local = (b->config->max_threads_per_node + ppn - 1) / ppn;
		int max_global = (b->config->max_global_threads + gt - 1) / gt;

		if (b->config->data_accessibility == ESDM_ACCESSIBILITY_GLOBAL){
			b->threads = max_local < max_global ? max_local : max_global;
		}else{
			b->threads = max_local;
		}
		DEBUG("Using %d threads for backend %s", b->threads, b->config->id);

    if (b->threads == 0){
      b->threadPool = NULL;
    }else{
      b->threadPool = g_thread_pool_new((GFunc)(backend_thread), b, b->threads, 1, & error);
    }
  }

	return scheduler;
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

static void backend_thread(io_work_t* work, esdm_backend_t* backend){
  io_request_status_t * status = work->parent;

  DEBUG("Backend thread operates on %s via %s", backend->name, backend->config->target);

  assert(backend == work->fragment->backend);

  esdm_status_t ret;
  switch(work->op){
    case(ESDM_OP_READ):{
      ret = esdm_fragment_retrieve(work->fragment);
      break;
    }
    case(ESDM_OP_WRITE):{
      ret = esdm_fragment_commit(work->fragment);
      break;
    }
  }

  work->return_code = ret;

	if(work->callback){
		work->callback(work);
	}

  g_mutex_lock(& status->mutex);
  status->pending_ops--;
  assert(status->pending_ops >= 0);
  if( status->pending_ops == 0){
    g_cond_signal(& status->done_condition);
  }
  g_mutex_unlock(& status->mutex);
	//esdm_dataspace_destroy(work->fragment->dataspace);
  free(work);
}

static void read_copy_callback(io_work_t * work){
	if(work->return_code != ESDM_SUCCESS){
		DEBUG("Error reading from fragment ", work->fragment);
		return;
	}
	char * b = (char*) work->data.mem_buf;
	esdm_dataspace_t * bs = work->data.buf_space;

	esdm_fragment_t * f = work->fragment;
	esdm_dataspace_t * fs = f->dataspace;
	//esdm_fragment_print(f);
	//printf("\n");
	assert(bs->datatype == fs->datatype);

	//calculate where to copy the fetched data to
	uint64_t size = esdm_sizeof(bs->datatype);
	// choose the dimension to split
	int split_dim = 0;
	for(int i=0; i < fs->dimensions; i++){
		if (fs->size[i] != 1){
			split_dim = i;
			break;
		}
	}

	//TODO proper serialization
	for(int d=0; d < fs->dimensions; d++){
		if (d != split_dim){
			size *= fs->size[d];
		}
	}
	uint64_t mn = bs->size[split_dim] > fs->size[split_dim] ? fs->size[split_dim] : bs->size[split_dim];
	uint64_t offset_mem = (fs->offset[split_dim] - bs->offset[split_dim]) * size;
	uint64_t offset_f = 0;

	size *= mn;

	DEBUG("SIZE: %ld %ld %ld", size, offset_mem, offset_f);

	// first dimension can be different:
	memcpy(b + offset_mem, ((char*)f->buf) + offset_f, size);

	free(f->buf);
}

esdm_status_t esdm_scheduler_enqueue_read(esdm_instance_t *esdm, io_request_status_t * status, int frag_count, esdm_fragment_t** read_frag, void * buf, esdm_dataspace_t * buf_space){
	GError * error;
	esdm_status_t ret;
	status->pending_ops += frag_count;

	int i, x;
	for (i = 0; i < frag_count; i++){
		esdm_fragment_t * f = read_frag[i];
		json_t *root = load_json(f->metadata->json);
		json_t * elem;
		elem = json_object_get(root, "plugin");
		const char * plugin_type = json_string_value(elem);
		elem = json_object_get(root, "id");
		const char * plugin_id = json_string_value(elem);
		elem = json_object_get(root, "offset");
		const char * offset_str = json_string_value(elem);
		elem = json_object_get(root, "size");
		const char * size_str = json_string_value(elem);

		if(!plugin_id){
			printf("Backend ID needs to be given\n");
			exit(1);
		}

		// find the backend for the fragment
		esdm_backend_t* backend_to_use = NULL;
		for (x = 0; x < esdm->modules->backend_count; x++){
			esdm_backend_t* b_tmp = esdm->modules->backends[x];
			if (strcmp(b_tmp->config->id, plugin_id) == 0){
				DEBUG("Found plugin %s", plugin_id);
				backend_to_use = b_tmp;
				break;
			}
		}
		if(backend_to_use == NULL){
			printf("Error no backend found for ID: %s\n", plugin_id);
			exit(1);
		}

		// esdm_fragment_print(read_frag[i]);
		// printf("\n");

		DEBUG("OFFSET/SIZE: %s %s\n", offset_str, size_str);

		//for verification purposes, we could read back the metadata stored and compare it...
		//esdm_dataspace_t * space = NULL;
		//ret = esdm_dataspace_overlap_str(parent_space, 'x', (char*)offset_str, (char*)size_str, & space);
		//assert(ret == ESDM_SUCCESS);

		uint64_t size = esdm_dataspace_size(f->dataspace);
		//printf("SIZE: %ld\n", size);
		f->backend = backend_to_use;

		io_work_t * task = (io_work_t*) malloc(sizeof(io_work_t));
		task->parent = status;
		task->op = ESDM_OP_READ;
		task->fragment = f;
		if(f->in_place){
			DEBUG("inplace!", "");
			task->callback = NULL;
			f->buf = buf;
		}else{
			f->buf = malloc(size);
			task->callback = read_copy_callback;
			task->data.mem_buf = buf;
			task->data.buf_space = buf_space;
		}
		if (backend_to_use->threads == 0){
			backend_thread(task, backend_to_use);
		}else{
			g_thread_pool_push(backend_to_use->threadPool, task, & error);
		}
	}

	return ESDM_SUCCESS;
}

esdm_status_t esdm_scheduler_enqueue_write(esdm_instance_t *esdm, io_request_status_t * status, esdm_dataset_t *dataset, void *buf,  esdm_dataspace_t* space){
    GError * error;
    //Gather I/O recommendations
    //esdm_performance_recommendation(esdm, NULL, NULL);    // e.g., split, merge, replication?
    //esdm_layout_recommendation(esdm, NULL, NULL);		  // e.g., merge, split, transform?

	// choose the dimension to split
	int split_dim = 0;
	for(int i=0; i < space->dimensions; i++){
		if (space->size[i] != 1){
			split_dim = i;
			break;
		}
	}

	// how big is one sub-hypercube? we call it y axis for the easier reading
	uint64_t one_y_size = 1;
	for (int i = 0; i < space->dimensions; i++)
	{
		if(i != split_dim){
			one_y_size *= space->size[i];
		}
	}
	one_y_size *= esdm_sizeof(space->datatype);

	if (one_y_size == 0){
		return ESDM_SUCCESS;
	}

	uint64_t y_count = space->size[split_dim];
	uint64_t per_backend[esdm->modules->backend_count];

	memset(per_backend, 0, sizeof(per_backend));

	while(y_count > 0){
		for (int i = 0; i < esdm->modules->backend_count; i++) {
			status->pending_ops++;
			esdm_backend_t* b = esdm->modules->backends[i];
			// how many of these fit into our buffer
			uint64_t backend_y_per_buffer = b->config->max_fragment_size / one_y_size;
			if (backend_y_per_buffer == 0){
				backend_y_per_buffer = 1;
			}
			if (backend_y_per_buffer >= y_count){
				per_backend[i] += y_count;
				y_count = 0;
				break;
			}else{
				per_backend[i] += backend_y_per_buffer;
				y_count -= backend_y_per_buffer;
			}
		}
	}
	ESDM_DEBUG_FMT("Will submit %d operations and for backend0: %d y-blocks", status->pending_ops, per_backend[0]);

	uint64_t offset_y = 0;
	int64_t dim[space->dimensions];
	int64_t offset[space->dimensions];
	memcpy(offset, space->offset, space->dimensions * sizeof(int64_t));
	memcpy(dim, space->size, space->dimensions * sizeof(int64_t));

	for (int i = 0; i < esdm->modules->backend_count; i++) {
		esdm_backend_t* b = esdm->modules->backends[i];
		// how many of these fit into our buffer
		uint64_t backend_y_per_buffer = b->config->max_fragment_size / one_y_size;
		if (backend_y_per_buffer == 0){
			backend_y_per_buffer = 1;
		}

		uint64_t y_total_access = per_backend[i];
		while(y_total_access > 0){
			uint64_t y_to_access = y_total_access > backend_y_per_buffer ? backend_y_per_buffer : y_total_access ;
			y_total_access -= y_to_access;

			dim[split_dim] = y_to_access;
			offset[split_dim] = offset_y + space->offset[split_dim];

			io_work_t * task = (io_work_t*) malloc(sizeof(io_work_t));
			esdm_dataspace_t* subspace = esdm_dataspace_subspace(space, space->dimensions, dim, offset);

			task->parent = status;
			task->op = ESDM_OP_WRITE;
			task->fragment = esdm_fragment_create(dataset, subspace, (char*) buf + offset_y * one_y_size);
			task->fragment->backend = b;
			task->callback = NULL;
			if (b->threads == 0){
				backend_thread(task, b);
			}else{
				g_thread_pool_push(b->threadPool, task, & error);
			}

			offset_y += y_to_access;
		}
	}

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

esdm_status_t esdm_scheduler_process_blocking(esdm_instance_t *esdm, io_operation_t op, esdm_dataset_t *dataset, void *buf,  esdm_dataspace_t* subspace){
	ESDM_DEBUG(__func__);

  io_request_status_t status;

	esdm_status_t ret;

  ret = esdm_scheduler_status_init(& status);
  assert( ret == ESDM_SUCCESS );

	esdm_fragment_t ** read_frag = NULL;
	int frag_count;

	if( op == ESDM_OP_WRITE){
		ret = esdm_scheduler_enqueue_write(esdm, & status, dataset, buf, subspace);
	}else if(op == ESDM_OP_READ){
		esdm_backend_t * md = esdm->modules->metadata;
		ret = md->callbacks.lookup(md, dataset, subspace, & frag_count, & read_frag);
		DEBUG("fragments to read: %d", frag_count);
		ret = esdm_scheduler_enqueue_read(esdm, & status, frag_count, read_frag, buf, subspace);
	}else{
		assert(0 && "Unknown operation");
	}
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
