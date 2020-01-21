#define _GNU_SOURCE

#include <esdm-internal.h>
#include <stdlib.h>
#include <test/util/test_util.h>

static double gFragmentAddTime = 0.0, gMakeSetTime = 0.0;
static int64_t gFragmentAddCount = 0, gMakeSetCount = 0;

static struct esdm_fragments_vtable_t gVtable;  //tentative definition, actual definition is at the end of the file

void esdmI_neighbourFragments_construct(struct esdmI_neighbourFragments_t* me) {
  *me = (struct esdmI_neighbourFragments_t){
    .super = {
      .vtable = &gVtable
    }
  };
}

static void add(void* meArg, esdm_fragment_t* fragment) {
  struct esdmI_neighbourFragments_t* me = meArg;

  timer myTimer;
  start_timer(&myTimer);

  if(me->count == me->buff_size) {
    me->buff_size = (me->buff_size ? 2*me->buff_size : 8);
    eassert(me->buff_size > me->count);
    me->frag = realloc(me->frag, me->buff_size * sizeof(*me->frag));
  }
  me->frag[me->count++] = fragment;

  esdmI_hypercube_t* extends;
  esdmI_dataspace_getExtends(fragment->dataspace, &extends);
  if(!me->neighbourManager) me->neighbourManager = esdmI_hypercubeNeighbourManager_make(esdmI_hypercube_dimensions(extends));
  esdmI_hypercubeNeighbourManager_pushBack(me->neighbourManager, extends);

  gFragmentAddTime += stop_timer(myTimer);
  gFragmentAddCount++;
}

static esdm_fragment_t** list(void* meArg, int64_t* out_fragmentCount) {
  struct esdmI_neighbourFragments_t* me = meArg;

  eassert(out_fragmentCount && "How can you know how many fragments there are, if you don't let me tell you?");

  *out_fragmentCount = me->count;
  return me->frag;
}

enum {
  NOT_VISITED = 0,  //the fragment has not been considered yet
  IN_FRONT, //the fragment has been scheduled for consideration
  SELECTED, //the fragment has been considered and was selected to be read
  IGNORED //the fragment has been considered and was deemed useless
};

static int compareSimilarities(const void* a, const void* b, void* aux) {
  const int64_t* leftIndex = a;
  const int64_t* rightIndex = b;
  double* similarities = aux;

  double diff = similarities[*leftIndex] - similarities[*rightIndex];
  return diff < 0 ? -1 :
         diff > 0 ? 1 : 0;
}

static esdm_fragment_t** makeSetCoveringRegion(void* meArg, esdmI_hypercube_t* bounds, int64_t* out_fragmentCount) {
  struct esdmI_neighbourFragments_t* me = meArg;

  eassert(me);
  eassert(bounds);
  eassert(out_fragmentCount);

  timer myTimer;
  start_timer(&myTimer);

  *out_fragmentCount = 0;
  esdm_fragment_t** fragmentSet = NULL;
  if(me->count) { //We need at least one fragment to know the dimension count, which we need to construct the neighbour manager.
    //Compute the neighbourhood info and check which fragments can be ignored because they do not provide any useful data.
    esdmI_hypercubeList_t* extendsList = esdmI_hypercubeNeighbourManager_list(me->neighbourManager);
    uint8_t* visited = malloc(me->count*sizeof(*visited));
    for(int64_t i = 0; i < me->count; i++) {
      visited[i] = (esdmI_hypercube_doesIntersect(bounds, extendsList->cubes[i]) ? NOT_VISITED : IGNORED);
    }

    //Compute the similarities of the fragments with the bounds and cache their extends.
    double* similarities = malloc(me->count*sizeof(*similarities));
    double bestSimilarity = -1;
    int64_t bestOverlap = -1, bestIndex = -1;
    for(int64_t i = 0; i < me->count; i++) {
      if(visited[i] == IGNORED) continue;
      similarities[i] = esdmI_hypercube_shapeSimilarity(bounds, extendsList->cubes[i]);
      if(similarities[i] > bestSimilarity) {
        bestSimilarity = similarities[i];
        bestOverlap = esdmI_hypercube_overlap(bounds, extendsList->cubes[i]);
        bestIndex = i;
      } else if(similarities[i] == bestSimilarity) {
        //In case of a tie in similarity, we prefer the cube with the largest overlap.
        int64_t overlap = esdmI_hypercube_overlap(bounds, extendsList->cubes[i]);
        if(overlap > bestOverlap) {
          bestOverlap = esdmI_hypercube_overlap(bounds, extendsList->cubes[i]);
          bestIndex = i;
        }
      }
    }

    if(bestIndex >= 0) { //We need at least one fragment where we can start the neighbour search.
      //Walk the neighbours of the selected fragment.
      //The way this is implemented, it performs a depth-first search, which is aborted immediately when the entire region has been covered.
      esdmI_hypercubeSet_t uncovered;
      esdmI_hypercubeSet_construct(&uncovered);
      esdmI_hypercubeSet_add(&uncovered, bounds);
      visited[bestIndex] = IN_FRONT;
      int64_t* front = malloc(me->count*sizeof(*front));  //the indices of the nodes to visit
      int64_t frontSize = 0, selectedFragmentCount = 0;
      front[frontSize++] = bestIndex;
      while(true) {
        bool done;
        for(; frontSize; ) {
          int64_t curIndex = front[--frontSize];
          esdmI_hypercube_t* curCube = extendsList->cubes[curIndex];
          if(esdmI_hypercubeList_doesIntersect(esdmI_hypercubeSet_list(&uncovered), curCube)) {
            visited[curIndex] = SELECTED;
            selectedFragmentCount++;
            esdmI_hypercubeSet_subtract(&uncovered, curCube);
            if((done = esdmI_hypercubeSet_isEmpty(&uncovered))) break; //Fast abort of search when we have all data we need.
            int64_t neighbourCount;
            int64_t* neighbours = esdmI_hypercubeNeighbourManager_getNeighbours(me->neighbourManager, curIndex, &neighbourCount);
            for(int64_t i = 0; i < neighbourCount; i++) {
              int64_t curNeighbour = neighbours[i];
              if(!visited[curNeighbour]) {
                visited[curNeighbour] = IN_FRONT;
                front[frontSize++] = curNeighbour;
                qsort_r(front, frontSize, sizeof(*front), compareSimilarities, similarities);
              }
            }
          } else {
            visited[curIndex] = IGNORED;
          }
        }
        if(done || esdmI_hypercubeSet_isEmpty(&uncovered)) break;

        //The available set of neighbours did not fully cover the bounds.
        //Search for a (non-neighbour) cube that still contributes to the uncovered region and restart the neighbour search.
        esdmI_hypercubeList_t* uncoveredList = esdmI_hypercubeSet_list(&uncovered);
        for(int64_t i = 0; i < me->count; i++) {
          if(visited[i]) continue;
          if(!esdmI_hypercubeList_doesIntersect(uncoveredList, extendsList->cubes[i])) continue;
          visited[i] = IN_FRONT;
          front[frontSize++] = i;
          break;
        }
        if(!frontSize) break; //The uncovered region does not intersect with any fragments that we did not consider yet -> caller will need to handle incomplete read.
      }

      //Filter the list of fragments by the selected subset
      fragmentSet = malloc(selectedFragmentCount*sizeof(*fragmentSet));
      for(int64_t i = 0; i < me->count; i++) {
        if(visited[i] == SELECTED) fragmentSet[(*out_fragmentCount)++] = me->frag[i];
      }

      //cleanup
      free(front);
      free(visited);
      esdmI_hypercubeSet_destruct(&uncovered);
      free(similarities);
    }
  }

  gMakeSetTime = stop_timer(myTimer);
  gMakeSetCount++;

  return fragmentSet;
}

static void metadata_create(void* meArg, smd_string_stream_t* s) {
  struct esdmI_neighbourFragments_t* me = meArg;

  smd_string_stream_printf(s, "[");
  for(int i=0; i < me->count; i++){
    if(i) smd_string_stream_printf(s, ",\n");
    esdm_fragment_metadata_create(me->frag[i], s);
  }
  smd_string_stream_printf(s, "]");
}

static esdm_status destruct(void* meArg) {
  struct esdmI_neighbourFragments_t* me = meArg;

  esdm_status result = ESDM_SUCCESS;
  for(int i = me->count; i--; ) {
    esdm_status ret = esdm_fragment_destroy(me->frag[i]);
    if(ret != ESDM_SUCCESS) result = ret;
  }
  free(me->frag);
  if(me->neighbourManager) esdmI_hypercubeNeighbourManager_destroy(me->neighbourManager);

  return result;
}

void esdmI_fragments_getStats(int64_t* out_addedFragments, double* out_fragmentAddTime, int64_t* out_createdSets, double* out_setCreationTime) {
  if(out_addedFragments) *out_addedFragments = gFragmentAddCount;
  if(out_fragmentAddTime) *out_fragmentAddTime = gFragmentAddTime;
  if(out_createdSets) *out_createdSets = gMakeSetCount;
  if(out_setCreationTime) *out_setCreationTime = gMakeSetTime;
}

double esdmI_fragments_getFragmentAddTime() { return gFragmentAddTime; }
int64_t esdmI_fragments_getFragmentAddCount() { return gFragmentAddCount; }
double esdmI_fragments_getSetCreationTime() { return gMakeSetTime; }
int64_t esdmI_fragments_getSetCreationCount() { return gMakeSetCount; }
void esdmI_fragments_resetStats() {
  gFragmentAddTime = gMakeSetTime = 0;
  gFragmentAddCount = gMakeSetCount = 0;
}

static struct esdm_fragments_vtable_t gVtable = {
  .add = add,
  .list = list,
  .makeSetCoveringRegion = makeSetCoveringRegion,
  .metadata_create = metadata_create,
  .destruct = destruct
};
