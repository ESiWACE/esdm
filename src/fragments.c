#define _GNU_SOURCE

#include <esdm-internal.h>
#include <stdlib.h>
#include <test/util/test_util.h>

static esdm_fragmentsTimes_t gStats;
esdm_fragmentsTimes_t esdmI_performance_fragments() { return gStats; }

static guint esdmI_fragments_hashKey(gconstpointer keyArg) {
  const esdmI_hypercube_t* key = keyArg;
  return esdmI_hypercube_hash(key);
}

static gboolean esdmI_fragments_equalKeys(gconstpointer keyArg1, gconstpointer keyArg2) {
  const esdmI_hypercube_t* key1 = keyArg1;
  const esdmI_hypercube_t* key2 = keyArg2;

  return esdmI_hypercube_equal(key1, key2) ? TRUE : FALSE;
}

static void esdmI_fragments_deallocateKey(gpointer keyArg) {
  esdmI_hypercube_t* key = keyArg;
  esdmI_hypercube_destroy(key);
}

static void esdmI_fragments_deallocateValue(gpointer valueArg) {
  esdm_fragment_t* fragment = valueArg;
  esdm_status result = esdm_fragment_destroy(fragment);
  eassert(result == ESDM_SUCCESS);
}

void esdmI_fragments_construct(esdm_fragments_t* me) {
  me->table = g_hash_table_new_full(esdmI_fragments_hashKey, esdmI_fragments_equalKeys, esdmI_fragments_deallocateKey, esdmI_fragments_deallocateValue);
}

esdm_status esdmI_fragments_add(esdm_fragments_t* me, esdm_fragment_t* fragment) {
  eassert(me);
  eassert(me->table);
  eassert(fragment);

  timer myTimer;
  ea_start_timer(&myTimer);

  esdmI_hypercube_t* key;
  esdm_status result = esdmI_dataspace_getExtends(fragment->dataspace, &key);
  eassert(result == ESDM_SUCCESS);
  if(g_hash_table_contains(me->table, key)) {
    result = ESDM_INVALID_STATE_ERROR;
  } else {
    g_hash_table_insert(me->table, key, fragment);
  }

  gStats.fragmentAddCalls++;
  gStats.fragmentAdding += ea_stop_timer(myTimer);

  return result;
}

esdm_fragment_t* esdmI_fragments_lookupForShape(esdm_fragments_t* me, esdmI_hypercube_t* shape) {
  timer myTimer;
  ea_start_timer(&myTimer);

  esdm_fragment_t* result = g_hash_table_lookup(me->table, shape);

  gStats.fragmentLookupCalls++;
  gStats.fragmentLookup += ea_stop_timer(myTimer);

  return result;
}

typedef struct deleteFragmentsFromBackendState {
  esdm_status result;
} deleteFragmentsFromBackendState;

static gboolean esdmI_fragments_deleteFragmentsFromBackend(gpointer keyArg, gpointer valueArg, gpointer stateArg) {
  esdmI_hypercube_t* key = keyArg;
  esdm_fragment_t* value = valueArg;
  deleteFragmentsFromBackendState* state = stateArg;

  esdm_status result = esdmI_backend_fragment_delete(value->backend, value);
  if(state->result == ESDM_SUCCESS) state->result = result;
  return result == ESDM_SUCCESS ? TRUE : FALSE;
}

esdm_status esdmI_fragments_deleteAll(esdm_fragments_t* me) {
  deleteFragmentsFromBackendState state = { .result = ESDM_SUCCESS };
  g_hash_table_foreach_remove(me->table, esdmI_fragments_deleteFragmentsFromBackend, &state);
  return state.result;
}

typedef struct selectFragmentsInRegionState {
  esdmI_hypercube_t* region;
  esdm_fragment_t** fragments;
  int64_t fragmentCount, bufferSize;
} selectFragmentsInRegionState;

static void esdmI_fragments_selectFragmentsInRegion(gpointer keyArg, gpointer valueArg, gpointer stateArg) {
  esdmI_hypercube_t* key = keyArg;
  esdm_fragment_t* value = valueArg;
  selectFragmentsInRegionState* state = stateArg;

  if(esdmI_hypercube_overlap(key, state->region)) {
    if(state->fragmentCount == state->bufferSize) {
      state->fragments = ea_checked_realloc(state->fragments, (state->bufferSize *= 2)*sizeof*state->fragments);
    }
    eassert(state->fragmentCount < state->bufferSize);
    state->fragments[state->fragmentCount++] = value;
  }
}

esdm_fragment_t** esdmI_fragments_makeSetCoveringRegion(esdm_fragments_t* me, esdmI_hypercube_t* bounds, int64_t* out_fragmentCount) {
  eassert(me);
  eassert(bounds);
  eassert(out_fragmentCount);

  timer myTimer;
  ea_start_timer(&myTimer);

  //try to find an exact match
  esdm_fragment_t* singleFragment = esdmI_fragments_lookupForShape(me, bounds);
  esdm_fragment_t** result = NULL;
  if(singleFragment) {
    *out_fragmentCount = 1;
    result = ea_memdup(&singleFragment, sizeof(singleFragment));
  } else {
    //search the hash table for matching fragments
    selectFragmentsInRegionState state = {
      .region = bounds,
      .fragmentCount = 0,
      .bufferSize = 8,
      .fragments = malloc(8*sizeof*state.fragments)
    };
    g_hash_table_foreach(me->table, esdmI_fragments_selectFragmentsInRegion, &state);

    //TODO: kick out any fragments that happen to be redundant

    *out_fragmentCount = state.fragmentCount;
    result = state.fragments;
  }

  gStats.setCreationCalls++;
  gStats.setCreation += ea_stop_timer(myTimer);

  return result;
}

typedef struct createFragmentMetadataState {
  smd_string_stream_t* stream;
  bool needComma;
} createFragmentMetadataState;

static void esdmI_fragments_createFragmentMetadata(gpointer keyArg, gpointer valueArg, gpointer stateArg) {
  esdm_fragment_t* value = valueArg;
  createFragmentMetadataState* state = stateArg;

  if(state->needComma) smd_string_stream_printf(state->stream, ",\n");
  state->needComma = true;
  esdm_fragment_metadata_create(value, state->stream);
}

void esdmI_fragments_metadata_create(esdm_fragments_t* me, smd_string_stream_t* stream) {
  timer myTimer;
  ea_start_timer(&myTimer);

  smd_string_stream_printf(stream, "[");
  createFragmentMetadataState state = {
    .stream = stream,
    .needComma = false
  };
  g_hash_table_foreach(me->table, esdmI_fragments_createFragmentMetadata, &state);
  smd_string_stream_printf(stream, "]");

  gStats.metadataCreationCalls++;
  gStats.metadataCreation += ea_stop_timer(myTimer);;
}

void esdmI_fragments_purge(esdm_fragments_t* me) {
  g_hash_table_remove_all(me->table);
}

esdm_status esdmI_fragments_destruct(esdm_fragments_t* me) {
  g_hash_table_destroy(me->table);
  return ESDM_SUCCESS;
}
