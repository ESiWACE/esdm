/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY {

 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <esdm-internal.h>
#include <esdm-datatypes-internal.h>

#include <stdio.h>

#define printTime(stream, timeObject, member) do { \
  if(timeObject.member > 0.0) fprintf(stream, "\t\t" #member ": %.3fs\n", timeObject.member); \
} while(0)

esdm_readTimes_t esdmI_performance_read_add(const esdm_readTimes_t* a, const esdm_readTimes_t* b) {
  return (esdm_readTimes_t){
    .makeSet = a->makeSet + b->makeSet,
    .coverageCheck = a->coverageCheck + b->coverageCheck,
    .enqueue = a->enqueue + b->enqueue,
    .completion = a->completion + b->completion,
    .writeback = a->writeback + b->writeback,
    .total = a->total + b->total,
  };
}

esdm_readTimes_t esdmI_performance_read_sub(const esdm_readTimes_t* minuend, const esdm_readTimes_t* subtrahend) {
  return (esdm_readTimes_t){
    .makeSet = minuend->makeSet - subtrahend->makeSet,
    .coverageCheck = minuend->coverageCheck - subtrahend->coverageCheck,
    .enqueue = minuend->enqueue - subtrahend->enqueue,
    .completion = minuend->completion - subtrahend->completion,
    .writeback = minuend->writeback - subtrahend->writeback,
    .total = minuend->total - subtrahend->total,
  };
}

void esdmI_performance_read_print(FILE* stream, const esdm_readTimes_t* start, const esdm_readTimes_t* end) {
  esdm_readTimes_t diff = start ? esdmI_performance_read_sub(end, start) : *end;
  if(diff.total > 0.0) {
    fprintf(stream, "\tread:\n");
    printTime(stream, diff, makeSet);
    printTime(stream, diff, coverageCheck);
    printTime(stream, diff, enqueue);
    printTime(stream, diff, completion);
    printTime(stream, diff, writeback);
    printTime(stream, diff, total);
  }
}

esdm_writeTimes_t esdmI_performance_write_add(const esdm_writeTimes_t* a, const esdm_writeTimes_t* b) {
  return (esdm_writeTimes_t){
    .backendDistribution = a->backendDistribution + b->backendDistribution,
    .backendDispatch = a->backendDispatch + b->backendDispatch,
    .completion = a->completion + b->completion,
    .total = a->total + b->total,
  };
}

esdm_writeTimes_t esdmI_performance_write_sub(const esdm_writeTimes_t* minuend, const esdm_writeTimes_t* subtrahend) {
  return (esdm_writeTimes_t){
    .backendDistribution = minuend->backendDistribution - subtrahend->backendDistribution,
    .backendDispatch = minuend->backendDispatch - subtrahend->backendDispatch,
    .completion = minuend->completion - subtrahend->completion,
    .total = minuend->total - subtrahend->total,
  };
}

void esdmI_performance_write_print(FILE* stream, const esdm_writeTimes_t* start, const esdm_writeTimes_t* end) {
  esdm_writeTimes_t diff = start ? esdmI_performance_write_sub(end, start) : *end;
  if(diff.total > 0.0) {
    fprintf(stream, "\twrite:\n");
    printTime(stream, diff, backendDistribution);
    printTime(stream, diff, backendDispatch);
    printTime(stream, diff, completion);
    printTime(stream, diff, total);
  }
}

esdm_copyTimes_t esdmI_performance_copy_add(const esdm_copyTimes_t* a, const esdm_copyTimes_t* b) {
  return (esdm_copyTimes_t){
    .planning = a->planning + b->planning,
    .execution = a->execution + b->execution,
    .total = a->total + b->total,
  };
}

esdm_copyTimes_t esdmI_performance_copy_sub(const esdm_copyTimes_t* minuend, const esdm_copyTimes_t* subtrahend) {
  return (esdm_copyTimes_t){
    .planning = minuend->planning - subtrahend->planning,
    .execution = minuend->execution - subtrahend->execution,
    .total = minuend->total - subtrahend->total,
  };
}

void esdmI_performance_copy_print(FILE* stream, const esdm_copyTimes_t* start, const esdm_copyTimes_t* end) {
  esdm_copyTimes_t diff = start ? esdmI_performance_copy_sub(end, start) : *end;
  if(diff.total > 0.0) {
    fprintf(stream, "\tcopy:\n");
    printTime(stream, diff, planning);
    printTime(stream, diff, execution);
    printTime(stream, diff, total);
  }
}

esdm_backendTimes_t esdmI_performance_backend() {
  return (esdm_backendTimes_t){0};
}

esdm_backendTimes_t esdmI_performance_backend_add(const esdm_backendTimes_t* a, const esdm_backendTimes_t* b) {
  return (esdm_backendTimes_t){
    .finalize = a->finalize + b->finalize,
    .performance_estimate = a->performance_estimate + b->performance_estimate,
    .estimate_throughput = a->estimate_throughput + b->estimate_throughput,
    .fragment_create = a->fragment_create + b->fragment_create,
    .fragment_retrieve = a->fragment_retrieve + b->fragment_retrieve,
    .fragment_update = a->fragment_update + b->fragment_update,
    .fragment_delete = a->fragment_delete + b->fragment_delete,
    .fragment_metadata_create = a->fragment_metadata_create + b->fragment_metadata_create,
    .fragment_metadata_load = a->fragment_metadata_load + b->fragment_metadata_load,
    .fragment_metadata_free = a->fragment_metadata_free + b->fragment_metadata_free,
    .mkfs = a->mkfs + b->mkfs,
    .fsck = a->fsck + b->fsck,
    .fragment_write_stream_blocksize = a->fragment_write_stream_blocksize + b->fragment_write_stream_blocksize,
  };
}

esdm_backendTimes_t esdmI_performance_backend_sub(const esdm_backendTimes_t* minuend, const esdm_backendTimes_t* subtrahend) {
  return (esdm_backendTimes_t){
    .finalize = minuend->finalize - subtrahend->finalize,
    .performance_estimate = minuend->performance_estimate - subtrahend->performance_estimate,
    .estimate_throughput = minuend->estimate_throughput - subtrahend->estimate_throughput,
    .fragment_create = minuend->fragment_create - subtrahend->fragment_create,
    .fragment_retrieve = minuend->fragment_retrieve - subtrahend->fragment_retrieve,
    .fragment_update = minuend->fragment_update - subtrahend->fragment_update,
    .fragment_delete = minuend->fragment_delete - subtrahend->fragment_delete,
    .fragment_metadata_create = minuend->fragment_metadata_create - subtrahend->fragment_metadata_create,
    .fragment_metadata_load = minuend->fragment_metadata_load - subtrahend->fragment_metadata_load,
    .fragment_metadata_free = minuend->fragment_metadata_free - subtrahend->fragment_metadata_free,
    .mkfs = minuend->mkfs - subtrahend->mkfs,
    .fsck = minuend->fsck - subtrahend->fsck,
    .fragment_write_stream_blocksize = minuend->fragment_write_stream_blocksize - subtrahend->fragment_write_stream_blocksize,
  };
}

void esdmI_performance_backend_print(FILE* stream, const esdm_backendTimes_t* start, const esdm_backendTimes_t* end) {
  esdm_backendTimes_t diff = start ? esdmI_performance_backend_sub(end, start) : *end;

  fprintf(stream, "\tbackend:\n");
  printTime(stream, diff, finalize);
  printTime(stream, diff, performance_estimate);
  printTime(stream, diff, estimate_throughput);
  printTime(stream, diff, fragment_create);
  printTime(stream, diff, fragment_retrieve);
  printTime(stream, diff, fragment_update);
  printTime(stream, diff, fragment_delete);
  printTime(stream, diff, fragment_metadata_create);
  printTime(stream, diff, fragment_metadata_load);
  printTime(stream, diff, fragment_metadata_free);
  printTime(stream, diff, mkfs);
  printTime(stream, diff, fsck);
  printTime(stream, diff, fragment_write_stream_blocksize);
}
