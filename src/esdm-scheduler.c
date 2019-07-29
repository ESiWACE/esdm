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

  if(esdm->modules){
    for (int i = 0; i < esdm->modules->data_backend_count; i++) {
      esdm_backend_t *b = esdm->modules->data_backends[i];
      if (b->threadPool) {
        g_thread_pool_free(b->threadPool, 0, 1);
      }
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

static int64_t min_int64(int64_t a, int64_t b) {
  return a < b ? a : b;
}

static int64_t max_int64(int64_t a, int64_t b) {
  return a > b ? a : b;
}

static int64_t abs_int64(int64_t a) {
  return a > 0 ? a : -a;
}

/**
 * Generate some, hopefully dimensionally reduced, instructions to perform `memcpy()` calls.
 * This is a helper function that is used to implement both `esdm_dataspace_copy_data()`
 * and support `esdmI_scheduler_try_direct_io()` by determining how many `memcpy()` calls would actually be needed to perform the copy.
 *
 * @param [in] sourceSpace the data space for the source buffer
 * @param [in] destSpace the data space for the destination buffer
 * @param [out] instructionDims returns the number of dimensions over which we need to iterate, may be zero (single `memcpy()` call), or -1 (nothing to copy)
 * @param [out] sourceOffset the offset of the first data chunk to read
 * @param [out] destOffset the offset of the first data chunk to write
 * @param [out] chunkSize number of bytes to copy with each `memcpy()` call
 * @param [out] size[instructionDims] the count of slices for each of the remaining dimensions
 * @param [out] relativeSourceStride[instructionDims] relative strides for the source buffer
 * @param [out] relativeDestStride[instructionDims] relative strides for the destination buffer
 *
 * The relative strides are defined as follows:
 * If the next dimension has a size of n, the relative stride is the offset in bytes between the slice at (..., 0, n, ...) and (..., 1, 0, ...).
 * That is, after the next dimension has been incremented to the one-past-the-end element, the relative stride is simply added to get the first element of the next slice.
 *
 * Or, viewing it the other way round, we go from the normal stride to the relative stride by
 * a) converting to byte offsets, and b) by subtracting the size of each dimension from the previous stride.
 * I.e. if the normal stride is (100, 10, 1) and the size is (10, 10, 10), the relative stride is (0, 0, 1), assuming an element size of 1.
 * Likewise, if the normal stride is (1000, 42, 3) and the size is (10, 10, 10), the relative stride is (580, 12, 1).
 * If the elements were of size 8, the relative stride would be (4640, 96, 8).
 * The point of using this relative stride is, that we can remove all state from the inner loop except for two simple `char` pointers.
 */
static void esdmI_dataspace_copy_instructions(
    esdm_dataspace_t* sourceSpace,
    esdm_dataspace_t* destSpace,
    int64_t* out_instructionDims,
    int64_t* out_chunkSize,
    int64_t* out_sourceOffset,
    int64_t* out_destOffset,
    int64_t* out_size,  //actually an array of size `*out_instructionDims`, may be NULL
    int64_t* out_relSourceStride,  //actually an array of size `*out_instructionDims`, may be NULL
    int64_t* out_relDestStride  //actually an array of size `*out_instructionDims`, may be NULL
) {
  eassert(sourceSpace->dims == destSpace->dims);
  eassert(sourceSpace->type == destSpace->type);
  eassert(out_instructionDims);
  eassert(out_chunkSize);
  eassert(out_sourceOffset);
  eassert(out_destOffset);

  uint64_t dimensions = sourceSpace->dims;

  //determine the hypercube that contains the overlap between the two hypercubes
  //(the intersection of two hypercubes is also a hypercube)
  int64_t overlapOffset[dimensions];
  uint64_t overlapSize[dimensions];
  for(int64_t i = 0; i < dimensions; i++) {
    int64_t sourceX0 = sourceSpace->offset[i];
    int64_t sourceX1 = sourceX0 + sourceSpace->size[i];
    int64_t destX0 = destSpace->offset[i];
    int64_t destX1 = destX0 + destSpace->size[i];
    int64_t overlapX0 = max_int64(sourceX0, destX0);
    int64_t overlapX1 = min_int64(sourceX1, destX1);
    overlapOffset[i] = overlapX0;
    overlapSize[i] = overlapX1 - overlapX0;
    if(overlapX0 > overlapX1) {
      *out_instructionDims = -1; //overlap is empty => nothing to do
      return;
    }
  }

  //in case the stride fields are set to NULL, determine the effective strides
  int64_t sourceStride[dimensions], destStride[dimensions];
  esdm_dataspace_getEffectiveStride(sourceSpace, sourceStride);
  esdm_dataspace_getEffectiveStride(destSpace, destStride);

  //determine how much data we can move with memcpy() at a time
  int64_t dataPointerOffset = 0, memcpySize = 1;  //both are counts of fundamental elements, not bytes
  bool memcpyDims[dimensions];
  memset(memcpyDims, 0, sizeof(memcpyDims));
  while(true) {
    //search for a dimension with a stride that matches our current memcpySize
    int64_t curDim;
    for(curDim = dimensions; curDim--; ) {
      if(!memcpyDims[curDim] && sourceStride[curDim] == destStride[curDim]) {
        if(abs_int64(sourceStride[curDim]) == memcpySize) break;
      }
    }
    if(curDim >= 0) {
      //found a fitting dimension, update our parameters
      memcpyDims[curDim] = true;  //remember not to loop over this dimension when copying the data
      if(sourceStride[curDim] < 0) dataPointerOffset += memcpySize*(overlapSize[curDim] - 1); //When the stride is negative, the first byte belongs to the last slice of this dimension, not the first one. Remember that.
      memcpySize *= overlapSize[curDim];
      if(overlapSize[curDim] != sourceSpace->size[curDim] || overlapSize[curDim] != destSpace->size[curDim]) break; //cannot fuse other dimensions in a memcpy() call if this dimensions does not have a perfect match between the dataspaces
    } else break; //didn't find another suitable dimension for fusing memcpy() calls
  }

  //determine the order of the remaining dimensions
  bool selectedDims[dimensions];
  memcpy(selectedDims, memcpyDims, sizeof(selectedDims));
  int64_t memcpyDim2LogicalDim[dimensions];
  for(*out_instructionDims = 0; ; (*out_instructionDims)++) {
    int64_t bestDim = -1;
    int64_t jumpFactor = 0;
    for(int64_t dim = 0; dim < dimensions; dim++) {
      if(!selectedDims[dim]) {
        int64_t curJumpFactor = min_int64(abs_int64(sourceStride[dim]), abs_int64(destStride[dim]));
        if(bestDim < 0 || jumpFactor < curJumpFactor) {
          bestDim = dim;
          jumpFactor = curJumpFactor;
        }
      }
    }
    if(bestDim >= 0) {
      selectedDims[bestDim] = true;
      memcpyDim2LogicalDim[*out_instructionDims] = bestDim;
    } else {
      break;
    }
  }

  //create the instruction data
  int64_t trashSize[dimensions], trashRelSourceStride[dimensions], trashRelDestStride[dimensions];
  if(!out_size) out_size = trashSize;
  if(!out_relSourceStride) out_relSourceStride = trashRelSourceStride;
  if(!out_relDestStride) out_relDestStride = trashRelDestStride;
  uint64_t elementSize = esdm_sizeof(sourceSpace->type);
  for(int64_t memcpyDim = 0; memcpyDim < *out_instructionDims; memcpyDim++) {
    int64_t logicalDim = memcpyDim2LogicalDim[memcpyDim];
    out_size[memcpyDim] = overlapSize[logicalDim];
    out_relSourceStride[memcpyDim] = sourceStride[logicalDim]*elementSize;
    out_relDestStride[memcpyDim] = destStride[logicalDim]*elementSize;
    if(memcpyDim) {
      //make the previous stride relative to this stride
      out_relSourceStride[memcpyDim-1] -= out_size[memcpyDim]*out_relSourceStride[memcpyDim];
      out_relDestStride[memcpyDim-1] -= out_size[memcpyDim]*out_relDestStride[memcpyDim];
    }
  }
  *out_chunkSize = memcpySize*elementSize;
  int64_t sourceIndex = 0, destIndex = 0;
  for(int64_t i = 0; i < dimensions; i++) {
    sourceIndex += (overlapOffset[i] - sourceSpace->offset[i])*sourceStride[i];
    destIndex += (overlapOffset[i] - destSpace->offset[i])*destStride[i];
  }
  *out_sourceOffset = (sourceIndex - dataPointerOffset)*elementSize;
  *out_destOffset = (destIndex - dataPointerOffset)*elementSize;
}

esdm_status esdm_dataspace_copy_data(esdm_dataspace_t* sourceSpace, void *voidPtrSource, esdm_dataspace_t* destSpace, void *voidPtrDest) {
  eassert(sourceSpace->dims == destSpace->dims);
  eassert(sourceSpace->type == destSpace->type);

  char* sourceData = voidPtrSource;
  char* destData = voidPtrDest;
  uint64_t dimensions = sourceSpace->dims;

  //get the instructions on how to copy the data
  int64_t instructionDims, chunkSize, sourceOffset, destOffset, size[dimensions], relSourceStride[dimensions], relDestStride[dimensions];
  esdmI_dataspace_copy_instructions(sourceSpace, destSpace, &instructionDims, &chunkSize, &sourceOffset, &destOffset, size, relSourceStride, relDestStride);

  //execute the instructions
  if(instructionDims < 0) return ESDM_SUCCESS;  //nothing to do
  sourceData += sourceOffset;
  destData += destOffset;
  int64_t counters[instructionDims];
  memset(counters, 0, sizeof(counters));
  while(true) {
    memcpy(destData, sourceData, chunkSize);

    int64_t i;
    for(i = instructionDims; i--; ) {
      sourceData += relSourceStride[i];
      destData += relDestStride[i];
      if(++(counters[i]) < size[i]) break;
      counters[i] = 0;
    }
    if(i == -1) break;
  }

  return ESDM_SUCCESS;
}

//FIXME: This has zero test coverage currently.
static void read_copy_callback(io_work_t *work) {
  if (work->return_code != ESDM_SUCCESS) {
    DEBUG("Error reading from fragment ", work->fragment);
    return;
  }
  esdm_dataspace_copy_data(work->fragment->dataspace, work->fragment->buf, work->data.buf_space, work->data.mem_buf);
  free(work->fragment->buf);  //TODO: Decide whether to cache the data.
  work->fragment->buf = NULL;
}

bool esdmI_scheduler_try_direct_io(esdm_fragment_t *f, void * buf, esdm_dataspace_t * da){
  eassert(f->dataspace->dims == da->dims);
  eassert(f->dataspace->type == da->type);

  uint64_t dimensions = f->dataspace->dims;

  int64_t instructionDims, chunkSize, sourceOffset, destOffset;
  esdmI_dataspace_copy_instructions(f->dataspace, da, &instructionDims, &chunkSize, &sourceOffset, &destOffset, NULL, NULL, NULL);
  if(instructionDims < 0) return true; //no overlap, nothing to do, we did it successfully
  if(instructionDims > 0) return false; //copy cannot be reduced to a single memcpy() call, so direct I/O is impossible

  //Ok, only a single memcpy() would be needed to move the data.
  //Determine whether the entire fragment's data would be needed.
  if(esdm_dataspace_size(f->dataspace) == chunkSize) {
    //The entire fragment's data is used in one piece.
    //Setup it's data buffers' pointer so that the backend will directly fill the correct part of the users' buffer.
    eassert(sourceOffset == 0 && "we have determined that we need the entire fragments data, so there should be no cut-off at the beginning");
    f->buf = (char*)buf + destOffset;
    return true;
  } else {
    //The fragments data is only used partially.
    //Thus, direct I/O is impossible as it would overwrite data next to the needed portion.
    return false;
  }
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
    if (esdmI_scheduler_try_direct_io(f, buf, buf_space)) {
      task->callback = NULL;
    } else {
      f->buf = malloc(size);  //This buffer will be filled by some background I/O process, read_copy_callback() will only be invoked *after* that has happened.
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

//Decide how the given dataset should be split into fragments to get sensible fragment sizes.
//Returns a hypercube set with one hypercube for each fragment that should be generated.
esdmI_hypercubeSet_t* esdm_scheduler_makeSplitRecommendation(esdm_instance_t *esdm, esdm_dataspace_t* space) {
  eassert(esdm);
  eassert(space);

  esdmI_hypercube_t* cube = esdmI_hypercube_make(space->dims, space->offset, space->size);
  esdmI_hypercubeSet_t* result = esdmI_hypercubeSet_make();
  esdmI_hypercubeSet_add(result, cube); //trivial implementation: just return the shape of the dataspace as a single hypercube.
  esdmI_hypercube_destroy(cube);
  return result;
}

//TODO Factor out the code to determine the split dimension, and make that new function also return the correct effective stride for the split dimension.
//     Refactor the rest of the function to allow splitting any dimension(s).
esdm_status esdm_scheduler_enqueue_write(esdm_instance_t *esdm, io_request_status_t *status, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *space) {
  GError *error;
  //Gather I/O recommendations
  //esdm_performance_recommendation(esdm, NULL, NULL);    // e.g., split, merge, replication?
  //esdm_layout_recommendation(esdm, NULL, NULL);		  // e.g., merge, split, transform?
  esdmI_hypercubeSet_t* cubes = esdm_scheduler_makeSplitRecommendation(esdm, space);

  int64_t dim[space->dims], offset[space->dims], stride[space->dims];
  for(int64_t i = 0, backendIndex = -1; i < cubes->count; i++) {
    status->pending_ops++;
    backendIndex = (backendIndex + 1)%esdm->modules->data_backend_count;
    esdm_backend_t* curBackend = esdm->modules->data_backends[backendIndex];
    eassert(curBackend);

    esdmI_hypercube_getOffsetAndSize(cubes->cubes[i], offset, dim);
    esdm_dataspace_getEffectiveStride(space, stride);

    esdm_dataspace_t* subspace;
    esdm_status ret = esdm_dataspace_subspace(space, space->dims, dim, offset, &subspace);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_set_stride(subspace, stride);
    eassert(ret == ESDM_SUCCESS);
    esdm_fragment_t* fragment;
    ret = esdmI_fragment_create(dataset, subspace, (char*)buf + esdm_dataspace_elementOffset(space, offset), &fragment);
    eassert(ret == ESDM_SUCCESS);
    fragment->backend = curBackend;

    io_work_t* task = malloc(sizeof(*task));
    *task = (io_work_t){
      .fragment = fragment,
      .op = ESDM_OP_WRITE,
      .return_code = ESDM_SUCCESS,
      .parent = status,
      .callback = NULL,
      .data = {NULL, NULL}
    };

    if (curBackend->threads == 0) {
      backend_thread(task, curBackend);
    } else {
      g_thread_pool_push(curBackend->threadPool, task, &error);
    }
  }

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
