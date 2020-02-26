#include <esdm-internal.h>
#include <esdm-datatypes-internal.h>

//decide which implementation to use
esdm_fragments_t* esdmI_fragments_create(esdm_dataset_t* parent) {
  esdm_fragments_t super = {
    .parent = parent
  };
  if(1) {
    struct esdmI_neighbourFragments_t* result = malloc(sizeof(*result));
    result->super = super;
    esdmI_neighbourFragments_construct(result);
    return &result->super;
  } else {
    struct esdmI_regularFragments_t* result = malloc(sizeof(*result));
    result->super = super;
    esdmI_regularFragments_construct(result, parent);
    return &result->super;
  }
  assert(0 && "this must be unreachable");
}

//virtual dispatch
void esdmI_fragments_add(esdm_fragments_t* me, esdm_fragment_t* fragment) { me->vtable->add(me, fragment); }
esdm_fragment_t** esdmI_fragments_list(esdm_fragments_t* me, int64_t* out_fragmentCount) { return me->vtable->list(me, out_fragmentCount); }
esdm_fragment_t** esdmI_fragments_makeSetCoveringRegion(esdm_fragments_t* me, esdmI_hypercube_t* region, int64_t* out_fragmentCount) { return me->vtable->makeSetCoveringRegion(me, region, out_fragmentCount); }
void esdmI_fragments_metadata_create(esdm_fragments_t* me, smd_string_stream_t* s) { me->vtable->metadata_create(me, s); }

//we provide a *_create(), so we need to provide a *_destroy() as well
esdm_status esdmI_fragments_destroy(esdm_fragments_t* me) {
  esdm_status result = me->vtable->destruct(me);
  free(me);
  return result;
}
