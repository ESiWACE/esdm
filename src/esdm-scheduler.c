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

/**
 * @file
 * @brief The scheduler receives application requests and schedules subsequent
 *        I/O requests as are necessary for metadata lookups and data
 *        reconstructions.
 *
 */

#include <esdm-internal.h>
#include <esdm.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("SCHEDULER", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("SCHEDULER", fmt, __VA_ARGS__)

static void backend_thread(io_work_t *data_p, esdm_backend_t *backend_id);

esdm_scheduler_t *esdm_scheduler_init(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  esdm_scheduler_t *scheduler = NULL;
  scheduler = (esdm_scheduler_t *)malloc(sizeof(esdm_scheduler_t));

  // create thread pools per device
  // decide how many threads should be used per backend.
  const int ppn = esdm->procs_per_node;
  const int gt = esdm->total_procs;
  GError *error;
  for (int i = 0; i < esdm->modules->data_backend_count; i++) {
    esdm_backend_t *b = esdm->modules->data_backends[i];
    // in total we should not use more than max_global total threads
    int max_local = (b->config->max_threads_per_node + ppn - 1) / ppn;
    int max_global = (b->config->max_global_threads + gt - 1) / gt;

    if (b->config->data_accessibility == ESDM_ACCESSIBILITY_GLOBAL) {
      b->threads = max_local < max_global ? max_local : max_global;
    } else {
      b->threads = max_local;
    }
    DEBUG("Using %d threads for backend %s", b->threads, b->config->id);

    if (b->threads == 0) {
      b->threadPool = NULL;
    } else {
      b->threadPool = g_thread_pool_new((GFunc)(backend_thread), b, b->threads, 1, &error);
    }
  }

  esdm->scheduler = scheduler;
  return scheduler;
}

esdm_status esdm_scheduler_finalize(esdm_instance_t *esdm) {
  ESDM_DEBUG(__func__);

  for (int i = 0; i < esdm->modules->data_backend_count; i++) {
    esdm_backend_t *b = esdm->modules->data_backends[i];
    if (b->threadPool) {
      g_thread_pool_free(b->threadPool, 0, 1);
    }
  }

  if (esdm->scheduler) {
    free(esdm->scheduler);
    esdm->scheduler = NULL;
  }

  return ESDM_SUCCESS;
}

static void backend_thread(io_work_t *work, esdm_backend_t *backend) {
  io_request_status_t *status = work->parent;

  DEBUG("Backend thread operates on %s via %s", backend->name, backend->config->target);

  eassert(backend == work->fragment->backend);

  esdm_status ret;
  switch (work->op) {
    case (ESDM_OP_READ): {
      ret = esdm_fragment_retrieve(work->fragment);
      break;
    }
    case (ESDM_OP_WRITE): {
      ret = esdm_fragment_commit(work->fragment);
      break;
    }
    default:
      ret = ESDM_ERROR;
  }

  work->return_code = ret;

  if (work->callback) {
    work->callback(work);
  }

  g_mutex_lock(&status->mutex);
  status->pending_ops--;
  if (ret != ESDM_SUCCESS){
    status->return_code = ret;
  }
  eassert(status->pending_ops >= 0);
  if (status->pending_ops == 0) {
    g_cond_signal(&status->done_condition);
  }
  g_mutex_unlock(&status->mutex);
  //esdm_dataspace_destroy(work->fragment->dataspace);

  work->fragment->status = ESDM_DATA_NOT_LOADED;
  free(work);
}

//FIXME: Make this stride aware!
static void read_copy_callback(io_work_t *work) {
  if (work->return_code != ESDM_SUCCESS) {
    DEBUG("Error reading from fragment ", work->fragment);
    return;
  }
  char *b = (char *)work->data.mem_buf;
  esdm_dataspace_t *bs = work->data.buf_space;

  esdm_fragment_t *f = work->fragment;
  esdm_dataspace_t *fs = f->dataspace;
  //esdm_fragment_print(f);
  //printf("\n");
  eassert(bs->type == fs->type);

  //calculate where to copy the fetched data to
  uint64_t size = esdm_sizeof(bs->type);
  // choose the dimension to split
  int split_dim = 0;
  for (int i = 0; i < fs->dims; i++) {
    if (fs->size[i] != 1) {
      // check if the split dimension is left
      if(i > 0 && fs->offset[i-1] != bs->offset[i-1]){
        split_dim = i - 1;
        break;
      }
      split_dim = i;
      break;
    }
  }

  //TODO proper serialization
  for (int d = 0; d < fs->dims; d++) {
    if (d != split_dim) {
      size *= fs->size[d];
    }
  }
  uint64_t mn = bs->size[split_dim] > fs->size[split_dim] ? fs->size[split_dim] : bs->size[split_dim];
  uint64_t offset_mem = (fs->offset[split_dim] - bs->offset[split_dim]) * size;
  uint64_t offset_f = 0;

  size *= mn;
  DEBUG("SIZE: %ld %ld %ld", size, offset_mem, offset_f);

  // first dimension can be different:
  memcpy(b + offset_mem, ((char *)f->buf) + offset_f, size);

  free(f->buf);
}

//FIXME: Make this stride aware!
int esdmI_scheduler_try_direct_io(esdm_fragment_t *f, void * buf, esdm_dataspace_t * da){
  esdm_dataspace_t * df = f->dataspace;
  int d;
  uint64_t size = 1;
  int pos = -1;
  for (d = da->dims - 1; d >= 0; d--) {
    int s1 = da->size[d];
    int s2 = df->size[d];
    // if it is out of bounds: abort
    if (s1 != s2){
      pos = d;
      break;
    }
    size *= s1;
  }
  // if it is a perfect match, the offsets must match, too
  if(d == -1){
    for (d = da->dims - 1; d >= 0; d--) {
      int o1 = da->offset[d];
      int o2 = df->offset[d];
      if (o1 != o2) break;
    }
    if( d == -1){
      // patches overlap perfectly => DIRECT IO
      f->buf = buf;
      return 1;
    }
    // partial overlap but same size
    return 0;
  }
  // verify that the dataspace is bigger than the fragment patch
  if(df->size[pos] > da->size[pos]){
    return 0;
  }
  // compute offset from patch to the buffer
  // check that all size/offsets are the same left from the current pos
  //FIXME: I think, this is a bug:
  //       If the fragment has dimensions (100, 1, 100) and the dataspace has dimensions (100, 100, 100), `pos` will be set to 1 at this point.
  //       The following loop will succeed because the first dimension has the same size.
  //       However, the data point (1, 0, 0) will be located at offset 100 in the fragment serialization, but at offset 10000 in the dataspace serialization.
  //       Thus, it cannot be written with a single I/O call, and this function should not return true.
  for (d = pos - 1; d >= 0; d--) {
    int s1 = da->size[d];
    int s2 = df->size[d];
    int o1 = da->offset[d];
    int o2 = df->offset[d];
    if (s1 != s2) return 0;
    if (o1 != o2) return 0;
  }
  // thus, the fragment is a real subset of the patch to access => DIRECT IO
  // finalize the computation of the identical dimensions
  size *= esdm_sizeof(df->type);
  char * newBuf = (char*) buf;
  newBuf += (df->offset[pos] - da->offset[pos]) * size;
  f->buf = newBuf;
  return 1;
}

esdm_status esdm_scheduler_enqueue_read(esdm_instance_t *esdm, io_request_status_t *status, int frag_count, esdm_fragment_t **read_frag, void *buf, esdm_dataspace_t *buf_space) {
  GError *error;

  status->pending_ops += frag_count;

  for (int i = 0; i < frag_count; i++) {
    esdm_fragment_t *f = read_frag[i];
    uint64_t size = esdm_dataspace_size(f->dataspace);
    esdm_backend_t *backend_to_use = f->backend;

    io_work_t *task = (io_work_t *)malloc(sizeof(io_work_t));
    task->parent = status;
    task->op = ESDM_OP_READ;
    task->fragment = f;
    //FIXME: I think, this is a bug:
    //       `read_copy_callback()` is written to copy data *out* of the fragment's buffer (makes sense to me),
    //       but `esdmI_scheduler_try_direct_io()` is written to assign the fragment's buffer *to `buf`*.
    //       Which appears to be the opposite direction of operation.
    if (esdmI_scheduler_try_direct_io(f, buf, buf_space)) {
      task->callback = NULL;
    } else {
      f->buf = malloc(size);
      task->callback = read_copy_callback;
      task->data.mem_buf = buf;
      task->data.buf_space = buf_space;
    }
    if (backend_to_use->threads == 0) {
      backend_thread(task, backend_to_use);
    } else {
      g_thread_pool_push(backend_to_use->threadPool, task, &error);
    }
  }

  return ESDM_SUCCESS;
}

//FIXME: Make this stride aware!
esdm_status esdm_scheduler_enqueue_write(esdm_instance_t *esdm, io_request_status_t *status, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *space) {
  GError *error;
  //Gather I/O recommendations
  //esdm_performance_recommendation(esdm, NULL, NULL);    // e.g., split, merge, replication?
  //esdm_layout_recommendation(esdm, NULL, NULL);		  // e.g., merge, split, transform?

  // choose the dimension to split
  int split_dim = 0;
  for (int i = 0; i < space->dims; i++) {
    if (space->size[i] != 1) {
      split_dim = i;
      break;
    }
  }

  // how big is one sub-hypercube? we call it y axis for the easier reading
  uint64_t one_y_size = 1;
  for (int i = 0; i < space->dims; i++) {
    if (i != split_dim) {
      one_y_size *= space->size[i];
    }
  }
  one_y_size *= esdm_sizeof(space->type);

  if (one_y_size == 0) {
    return ESDM_SUCCESS;
  }

  uint64_t y_count = space->size[split_dim];
  uint64_t per_backend[esdm->modules->data_backend_count];

  memset(per_backend, 0, sizeof(per_backend));

  while (y_count > 0) {
    for (int i = 0; i < esdm->modules->data_backend_count; i++) {
      status->pending_ops++;
      esdm_backend_t *b = esdm->modules->data_backends[i];
      // how many of these fit into our buffer
      uint64_t backend_y_per_buffer = b->config->max_fragment_size / one_y_size;
      if (backend_y_per_buffer == 0) {
        backend_y_per_buffer = 1;
      }
      if (backend_y_per_buffer >= y_count) {
        per_backend[i] += y_count;
        y_count = 0;
        break;
      } else {
        per_backend[i] += backend_y_per_buffer;
        y_count -= backend_y_per_buffer;
      }
    }
  }
  ESDM_DEBUG_FMT("Will submit %d operations and for backend0: %d y-blocks", status->pending_ops, per_backend[0]);

  uint64_t offset_y = 0;
  int64_t dim[space->dims];
  int64_t offset[space->dims];
  memcpy(offset, space->offset, space->dims * sizeof(int64_t));
  memcpy(dim, space->size, space->dims * sizeof(int64_t));

  for (int i = 0; i < esdm->modules->data_backend_count; i++) {
    esdm_backend_t *b = esdm->modules->data_backends[i];
    eassert(b);
    // how many of these fit into our buffer
    uint64_t backend_y_per_buffer = b->config->max_fragment_size / one_y_size;
    if (backend_y_per_buffer == 0) {
      backend_y_per_buffer = 1;
    }

    uint64_t y_total_access = per_backend[i];
    esdm_status ret;
    while (y_total_access > 0) {
      uint64_t y_to_access = y_total_access > backend_y_per_buffer ? backend_y_per_buffer : y_total_access;
      y_total_access -= y_to_access;

      dim[split_dim] = y_to_access;
      offset[split_dim] = offset_y + space->offset[split_dim];

      io_work_t *task = (io_work_t *)malloc(sizeof(io_work_t));
      esdm_dataspace_t *subspace;

      ret = esdm_dataspace_subspace(space, space->dims, dim, offset, &subspace);
      eassert(ret == ESDM_SUCCESS);
      task->parent = status;
      task->op = ESDM_OP_WRITE;
      ret = esdmI_fragment_create(dataset, subspace, (char *)buf + offset_y * one_y_size, &task->fragment);
      eassert(ret == ESDM_SUCCESS);
      task->fragment->backend = b;
      task->callback = NULL;
      if (b->threads == 0) {
        backend_thread(task, b);
      } else {
        g_thread_pool_push(b->threadPool, task, &error);
      }

      offset_y += y_to_access;
    }
  }

  // now enqueue the operations
  return ESDM_SUCCESS;
}

esdm_status esdm_scheduler_status_init(io_request_status_t *status) {
  g_mutex_init(&status->mutex);
  g_cond_init(&status->done_condition);
  status->pending_ops = 0;
  status->return_code = ESDM_SUCCESS;
  return ESDM_SUCCESS;
}

esdm_status esdm_scheduler_status_finalize(io_request_status_t *status) {
  g_mutex_clear(&status->mutex);
  g_cond_clear(&status->done_condition);
  return ESDM_SUCCESS;
}

esdm_status esdm_scheduler_wait(io_request_status_t *status) {
  g_mutex_lock(&status->mutex);
  if (status->pending_ops) {
    g_cond_wait(&status->done_condition, &status->mutex);
  }
  g_mutex_unlock(&status->mutex);
  return ESDM_SUCCESS;
}

esdm_status esdm_scheduler_process_blocking(esdm_instance_t *esdm, io_operation_t op, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *subspace) {
  ESDM_DEBUG(__func__);

  io_request_status_t status;

  esdm_status ret = esdm_scheduler_status_init(&status);
  eassert(ret == ESDM_SUCCESS);

  esdm_fragment_t **read_frag = NULL;
  int frag_count;

  if (op == ESDM_OP_WRITE) {
    ret = esdm_scheduler_enqueue_write(esdm, &status, dataset, buf, subspace);
  } else if (op == ESDM_OP_READ) {
    ret = esdmI_dataset_lookup_fragments(dataset, subspace, & frag_count, &read_frag);
    eassert(ret == ESDM_SUCCESS);
    DEBUG("fragments to read: %d", frag_count);
    ret = esdm_scheduler_enqueue_read(esdm, &status, frag_count, read_frag, buf, subspace);
  } else {
    eassert(0 && "Unknown operation");
  }
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_scheduler_wait(&status);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_scheduler_status_finalize(&status);
  eassert(ret == ESDM_SUCCESS);

  free(read_frag);

  return status.return_code;
}
