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

#define printTime(stream, linePrefix, indentation, timeObject, member) do { \
  if(timeObject.member > 0.0) fprintf(stream, "%s%s" #member ": %.3fs\n", linePrefix, indentation, timeObject.member); \
} while(0)

#define printCountedTime(stream, linePrefix, indentation, object, timeMember, countMember) do { \
  if(object.countMember > 0) fprintf(stream, "%s%s" #timeMember ": %"PRId64" calls in %.3fs (%.3fms per call)\n", linePrefix, indentation, object.countMember, object.timeMember, object.timeMember*1000/object.countMember); \
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

void esdmI_performance_read_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_readTimes_t* start, const esdm_readTimes_t* end) {
  esdm_readTimes_t diff = start ? esdmI_performance_read_sub(end, start) : *end;
  if(diff.total > 0.0) {
    fprintf(stream, "%sread:\n", linePrefix);
    printTime(stream, linePrefix, indentation, diff, makeSet);
    printTime(stream, linePrefix, indentation, diff, coverageCheck);
    printTime(stream, linePrefix, indentation, diff, enqueue);
    printTime(stream, linePrefix, indentation, diff, completion);
    printTime(stream, linePrefix, indentation, diff, writeback);
    printTime(stream, linePrefix, indentation, diff, total);
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

void esdmI_performance_write_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_writeTimes_t* start, const esdm_writeTimes_t* end) {
  esdm_writeTimes_t diff = start ? esdmI_performance_write_sub(end, start) : *end;
  if(diff.total > 0.0) {
    fprintf(stream, "%swrite:\n", linePrefix);
    printTime(stream, linePrefix, indentation, diff, backendDistribution);
    printTime(stream, linePrefix, indentation, diff, backendDispatch);
    printTime(stream, linePrefix, indentation, diff, completion);
    printTime(stream, linePrefix, indentation, diff, total);
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

void esdmI_performance_copy_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_copyTimes_t* start, const esdm_copyTimes_t* end) {
  esdm_copyTimes_t diff = start ? esdmI_performance_copy_sub(end, start) : *end;
  if(diff.total > 0.0) {
    fprintf(stream, "%scopy:\n", linePrefix);
    printTime(stream, linePrefix, indentation, diff, planning);
    printTime(stream, linePrefix, indentation, diff, execution);
    printTime(stream, linePrefix, indentation, diff, total);
  }
}

static esdm_backendTimes_t gBackendTimes = {0};

int esdmI_backend_finalize(esdm_backend_t * b) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.finalize(b);
  gBackendTimes.finalize += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_performance_estimate(esdm_backend_t * b, esdm_fragment_t *fragment, float *out_time) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.performance_estimate(b, fragment, out_time);
  gBackendTimes.performance_estimate += ea_stop_timer(clock);
  return result;
}

float esdmI_backend_estimate_throughput (esdm_backend_t * b) {
  timer clock;
  ea_start_timer(&clock);
  float result = b->callbacks.estimate_throughput (b);
  gBackendTimes.estimate_throughput += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_create (esdm_backend_t * b, esdm_fragment_t *fragment) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_create (b, fragment);
  gBackendTimes.fragment_create += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_retrieve(esdm_backend_t * b, esdm_fragment_t *fragment) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_retrieve(b, fragment);
  gBackendTimes.fragment_retrieve += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_update (esdm_backend_t * b, esdm_fragment_t *fragment) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_update (b, fragment);
  gBackendTimes.fragment_update += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_delete (esdm_backend_t * b, esdm_fragment_t *fragment) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_delete (b, fragment);
  gBackendTimes.fragment_delete += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_metadata_create(esdm_backend_t * b, esdm_fragment_t *fragment, smd_string_stream_t* stream) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_metadata_create(b, fragment,  stream);
  gBackendTimes.fragment_metadata_create += ea_stop_timer(clock);
  return result;
}

void* esdmI_backend_fragment_metadata_load(esdm_backend_t * b, esdm_fragment_t *fragment, json_t *metadata) {
  timer clock;
  ea_start_timer(&clock);
  void* result = b->callbacks.fragment_metadata_load(b, fragment, metadata);
  gBackendTimes.fragment_metadata_load += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_metadata_free (esdm_backend_t * b, void * options) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_metadata_free (b, options);
  gBackendTimes.fragment_metadata_free += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_mkfs(esdm_backend_t * b, int format_flags) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.mkfs(b, format_flags);
  gBackendTimes.mkfs += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fsck(esdm_backend_t * b) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fsck(b);
  gBackendTimes.fsck += ea_stop_timer(clock);
  return result;
}

int esdmI_backend_fragment_write_stream_blocksize(esdm_backend_t * b, estream_write_t * state, void * cur_buf, size_t cur_offset, uint32_t cur_size) {
  timer clock;
  ea_start_timer(&clock);
  int result = b->callbacks.fragment_write_stream_blocksize(b, state, cur_buf, cur_offset, cur_size);
  gBackendTimes.fragment_write_stream_blocksize += ea_stop_timer(clock);
  return result;
}

esdm_backendTimes_t esdmI_performance_backend() {
  return gBackendTimes;
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

void esdmI_performance_backend_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_backendTimes_t* start, const esdm_backendTimes_t* end) {
  esdm_backendTimes_t diff = start ? esdmI_performance_backend_sub(end, start) : *end;

  fprintf(stream, "%sbackend:\n", linePrefix);
  printTime(stream, linePrefix, indentation, diff, finalize);
  printTime(stream, linePrefix, indentation, diff, performance_estimate);
  printTime(stream, linePrefix, indentation, diff, estimate_throughput);
  printTime(stream, linePrefix, indentation, diff, fragment_create);
  printTime(stream, linePrefix, indentation, diff, fragment_retrieve);
  printTime(stream, linePrefix, indentation, diff, fragment_update);
  printTime(stream, linePrefix, indentation, diff, fragment_delete);
  printTime(stream, linePrefix, indentation, diff, fragment_metadata_create);
  printTime(stream, linePrefix, indentation, diff, fragment_metadata_load);
  printTime(stream, linePrefix, indentation, diff, fragment_metadata_free);
  printTime(stream, linePrefix, indentation, diff, mkfs);
  printTime(stream, linePrefix, indentation, diff, fsck);
  printTime(stream, linePrefix, indentation, diff, fragment_write_stream_blocksize);
}

esdm_fragmentsTimes_t esdmI_performance_fragments_add(const esdm_fragmentsTimes_t* a, const esdm_fragmentsTimes_t* b) {
  return (esdm_fragmentsTimes_t){
    .fragmentAdding = a->fragmentAdding + b->fragmentAdding,
    .fragmentLookup = a->fragmentLookup + b->fragmentLookup,
    .metadataCreation = a->metadataCreation + b->metadataCreation,
    .setCreation = a->setCreation + b->setCreation,
    .fragmentAddCalls = a->fragmentAddCalls + b->fragmentAddCalls,
    .fragmentLookupCalls = a->fragmentLookupCalls + b->fragmentLookupCalls,
    .metadataCreationCalls = a->metadataCreationCalls + b->metadataCreationCalls,
    .setCreationCalls = a->setCreationCalls + b->setCreationCalls,
  };
}

esdm_fragmentsTimes_t esdmI_performance_fragments_sub(const esdm_fragmentsTimes_t* minuend, const esdm_fragmentsTimes_t* subtrahend) {
  return (esdm_fragmentsTimes_t){
    .fragmentAdding = minuend->fragmentAdding - subtrahend->fragmentAdding,
    .fragmentLookup = minuend->fragmentLookup - subtrahend->fragmentLookup,
    .metadataCreation = minuend->metadataCreation - subtrahend->metadataCreation,
    .setCreation = minuend->setCreation - subtrahend->setCreation,
    .fragmentAddCalls = minuend->fragmentAddCalls - subtrahend->fragmentAddCalls,
    .fragmentLookupCalls = minuend->fragmentLookupCalls - subtrahend->fragmentLookupCalls,
    .metadataCreationCalls = minuend->metadataCreationCalls - subtrahend->metadataCreationCalls,
    .setCreationCalls = minuend->setCreationCalls - subtrahend->setCreationCalls,
  };
}

void esdmI_performance_fragments_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_fragmentsTimes_t* start, const esdm_fragmentsTimes_t* end) {
  esdm_fragmentsTimes_t diff = start ? esdmI_performance_fragments_sub(end, start) : *end;

  fprintf(stream, "%sfragment handling:\n", linePrefix);
  printCountedTime(stream, linePrefix, indentation, diff, fragmentAdding, fragmentAddCalls);
  printCountedTime(stream, linePrefix, indentation, diff, fragmentLookup, fragmentLookupCalls);
  printCountedTime(stream, linePrefix, indentation, diff, metadataCreation, metadataCreationCalls);
  printCountedTime(stream, linePrefix, indentation, diff, setCreation, setCreationCalls);
}
