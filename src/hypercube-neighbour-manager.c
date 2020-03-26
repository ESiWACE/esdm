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
 * @brief This file implements the hypercube neighbour manager, a class that exists to identify neighbourhood relations within a set of hypercubes,
 * and provide a quick way to look up all the neighbours of a given cube.
 */

#include <esdm-internal.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_BOUND_TREE false

// esdmI_boundArray_t ///////////////////////////////////////////////////////////////////////////////

static void boundArray_add(esdmI_boundArray_t* me, int64_t bound, bool isStart, int64_t cubeIndex);
static void boundArray_add_callStub(void* me, int64_t bound, bool isStart, int64_t cubeIndex) { boundArray_add(me, bound, isStart, cubeIndex); }

static esdmI_boundListEntry_t* boundArray_findFirst(esdmI_boundArray_t* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator);
static esdmI_boundListEntry_t* boundArray_findFirst_callStub(void* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator) { return boundArray_findFirst(me, bound, isStart, out_iterator); }

static esdmI_boundListEntry_t* boundArray_nextEntry(esdmI_boundArray_t* me, esdmI_boundIterator_t* inout_iterator);
static esdmI_boundListEntry_t* boundArray_nextEntry_callStub(void* me, esdmI_boundIterator_t* inout_iterator) { return boundArray_nextEntry(me, inout_iterator); }

static void boundArray_destruct(esdmI_boundArray_t* me);
static void boundArray_destruct_callStub(void* me) { boundArray_destruct(me); }

static esdmI_boundList_vtable_t boundArray_vtable = {
  .add = boundArray_add_callStub,
  .findFirst = boundArray_findFirst_callStub,
  .nextEntry = boundArray_nextEntry_callStub,
  .destruct = boundArray_destruct_callStub
};

static void boundArray_construct(esdmI_boundArray_t* me) {
  *me = (esdmI_boundArray_t){
    .super = { .vtable = &boundArray_vtable },
    .entries = NULL,
    .count = 0,
    .allocatedCount = 16
  };
  me->entries = malloc(me->allocatedCount*sizeof(*me->entries));
  eassert(me->entries);
}

//This is a no-fail lookup that always returns either the first entry with the given bound, or the place where such an entry should be inserted into the list.
//As such, it may return a pointer past the end of the array if the bound is larger than all existing entries.
static esdmI_boundListEntry_t* boundArray_lookup(esdmI_boundArray_t* me, int64_t bakedBound) {
  int64_t low = 0, high = me->count;
  while(low < high) {
    int64_t mid = (low + high)/2;
    eassert(mid < high);
    if(me->entries[mid].bakedBound < bakedBound) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return &me->entries[high];
}

static int64_t bakeBound(int64_t bound, bool isStart) {
  return 2*bound + !!isStart; //the !! is important to turn any internal representation of true into a 1
}

static void boundArray_add(esdmI_boundArray_t* me, int64_t bound, bool isStart, int64_t cubeIndex) {
  //make sure we have enough space
  if(me->count == me->allocatedCount) {
    me->allocatedCount *= 2;
    me->entries = realloc(me->entries, me->allocatedCount*sizeof(*me->entries));
    eassert(me->entries);
  }
  eassert(me->allocatedCount > me->count);

  int64_t bakedBound = bakeBound(bound, isStart);
  esdmI_boundListEntry_t* iterator = boundArray_lookup(me, bakedBound); //get the place where to insert the entry …
  memmove(iterator + 1, iterator, (me->entries + me->count++ - iterator)*sizeof(*iterator));  //… shift everything after that …
  *iterator = (esdmI_boundListEntry_t){ //… and insert the new entry
    .bakedBound = bakedBound,
    .cubeIndex = cubeIndex
  };
}

//find the first entry with the given bound, or return NULL if no such entry exists
static esdmI_boundListEntry_t* boundArray_findFirst(esdmI_boundArray_t* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator) {
  eassert(out_iterator);
  int64_t bakedBound = bakeBound(bound, isStart);
  esdmI_boundListEntry_t* result = boundArray_lookup(me, bakedBound);
  if(result - me->entries >= me->count) return NULL;
  if(result->bakedBound != bakedBound) return NULL;
  return out_iterator->arrayIterator.entry = result;
}

//returns the next entry with the same bound, or NULL
static esdmI_boundListEntry_t* boundArray_nextEntry(esdmI_boundArray_t* me, esdmI_boundIterator_t* inout_iterator) {
  eassert(inout_iterator);
  eassert(inout_iterator->arrayIterator.entry);

  int64_t nextIndex = inout_iterator->arrayIterator.entry - me->entries + 1;
  if(nextIndex >= me->count) return NULL;
  if(me->entries[nextIndex].bakedBound != inout_iterator->arrayIterator.entry->bakedBound) return NULL;
  return inout_iterator->arrayIterator.entry = &me->entries[nextIndex];
}

static void boundArray_destruct(esdmI_boundArray_t* me) {
  free(me->entries);
}

// esdmI_boundTree_t ///////////////////////////////////////////////////////////////////////////////

static void boundTree_add(esdmI_boundTree_t* me, int64_t bound, bool isStart, int64_t cubeIndex);
static void boundTree_add_callStub(void* me, int64_t bound, bool isStart, int64_t cubeIndex) { boundTree_add(me, bound, isStart, cubeIndex); }

static esdmI_boundListEntry_t*  boundTree_findFirst(esdmI_boundTree_t* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator);
static esdmI_boundListEntry_t*  boundTree_findFirst_callStub(void* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator) { return boundTree_findFirst(me, bound, isStart, out_iterator); }

static esdmI_boundListEntry_t* boundTree_nextEntry(esdmI_boundTree_t* me, esdmI_boundIterator_t* inout_iterator);
static esdmI_boundListEntry_t* boundTree_nextEntry_callStub(void* me, esdmI_boundIterator_t* inout_iterator) { return boundTree_nextEntry(me, inout_iterator); }

static void boundTree_destruct(esdmI_boundTree_t* me);
static void boundTree_destruct_callStub(void* me) { boundTree_destruct(me); }

static esdmI_boundList_vtable_t boundTree_vtable = {
  .add = boundTree_add_callStub,
  .findFirst = boundTree_findFirst_callStub,
  .nextEntry = boundTree_nextEntry_callStub,
  .destruct = boundTree_destruct_callStub
};

static void boundTree_construct(esdmI_boundTree_t* me) {
  *me = (esdmI_boundTree_t){
    .super = { .vtable = &boundTree_vtable }
    //other fields are zero initialized
  };
}

//For debugging purposes.
__attribute__((unused)) static void boundTree_print(esdmI_boundTree_t* me, FILE* stream, int indentation) {
  if(!me) return;

  fprintf(stream, "%*s{\n", 4*indentation++, "");
  for(int64_t i = 0; ; i++) {
    boundTree_print(me->children[i], stream, indentation);
    if(i >= me->entryCount) break;
    fprintf(stream, "%*sbakedBound = %"PRId64", index = %"PRId64"\n", 4*indentation, "", me->bounds[i].bakedBound, me->bounds[i].cubeIndex);
  }
  fprintf(stream, "%*s}\n", 4*--indentation, "");
}

//For debugging purposes.
static void boundTree_checkTree(esdmI_boundTree_t* me) {
  for(int64_t i = 0; i <= me->entryCount; i++) {
    if(me->children[i]) {
      eassert(me->children[i]->parent == me);
      boundTree_checkTree(me->children[i]);
    }
  }
}

//Update parent pointers within all direct children.
static void boundTree_fixRelations(esdmI_boundTree_t* me) {
  for(int64_t i = 0; i <= me->entryCount; i++) {
    if(me->children[i]) {
      me->children[i]->parent = me;
    }
  }
}

//Split a full node into two new nodes, reducing this node to an `entryCount == 1` node.
static void boundTree_splitNode_internal(esdmI_boundTree_t* me) {
  eassert(me->entryCount == BOUND_TREE_MAX_ENTRY_COUNT);
  if(DEBUG_BOUND_TREE) boundTree_checkTree(me);

  esdmI_boundTree_t* left = malloc(sizeof(*left));
  esdmI_boundTree_t* right = malloc(sizeof(*right));
  const int splitIndex = BOUND_TREE_MAX_ENTRY_COUNT/2;

  *left = (esdmI_boundTree_t){
    .entryCount = splitIndex,
    .parent = me
  };
  memmove(left->bounds, me->bounds, left->entryCount*sizeof(*left->bounds));
  memmove(left->children, me->children, (left->entryCount + 1)*sizeof(*left->children));
  boundTree_fixRelations(left);

  *right = (esdmI_boundTree_t){
    .entryCount = me->entryCount - splitIndex - 1,
    .parent = me
  };
  memmove(right->bounds, &me->bounds[splitIndex + 1], right->entryCount*sizeof(*right->bounds));
  memmove(right->children, &me->children[splitIndex + 1], (right->entryCount + 1)*sizeof(*right->children));
  boundTree_fixRelations(right);

  me->entryCount = 1;
  me->bounds[0] = me->bounds[splitIndex];
  me->children[0] = left;
  me->children[1] = right;
  if(DEBUG_BOUND_TREE) boundTree_checkTree(me);
}

//Returns the first position with a greater bakedBound than the given entry, i.e. the location where the newEntry should be inserted.
static int boundTree_findChildPosition(esdmI_boundTree_t* me, int64_t bakedBound) {
  int childIndex = 0;
  for(; childIndex < me->entryCount; childIndex++) {
    if(me->bounds[childIndex].bakedBound > bakedBound) break;
  }
  return childIndex;
}

//If there's no room left to add the entry, the node is split into two new nodes, which become the only two children of the old node.
//The old node is then returned to indicate the caller that it needs to add the two new nodes as its own children recursively.
//With enough space in the respective node, this returns NULL to indicate to the caller that no further work is necessary.
static esdmI_boundTree_t* boundTree_add_internal(esdmI_boundTree_t* me, esdmI_boundListEntry_t newEntry) {
  esdmI_boundTree_t* result = NULL;
  int childIndex = boundTree_findChildPosition(me, newEntry.bakedBound);
  if(me->children[childIndex]) {
    //This is an internal node, recurse.
    esdmI_boundTree_t* returnedNode = boundTree_add_internal(me->children[childIndex], newEntry);
    if(!returnedNode) return NULL;  //no more splitting necessary

    //Add the children of the returned node to this node, if possible.
    eassert(returnedNode->entryCount == 1);
    if(me->entryCount == BOUND_TREE_MAX_ENTRY_COUNT) {
      //Need to split this node as well.
      boundTree_splitNode_internal(me);
      result = me;
      //Continue as the child that should absorb the returnedNode's bound.
      me = me->children[me->bounds[0].bakedBound <= returnedNode->bounds[0].bakedBound];
      childIndex = boundTree_findChildPosition(me, returnedNode->bounds[0].bakedBound);
    }

    //Now that we have enough space here, adopt the children of the returnedNode.
    memmove(&me->bounds[childIndex + 1], &me->bounds[childIndex], (me->entryCount - childIndex)*sizeof(*me->bounds));
    memmove(&me->children[childIndex + 2], &me->children[childIndex + 1], (me->entryCount - childIndex)*sizeof(*me->children));
    me->bounds[childIndex] = returnedNode->bounds[0];
    me->children[childIndex] = returnedNode->children[0];
    me->children[childIndex]->parent = me;
    me->children[childIndex + 1] = returnedNode->children[1];
    me->children[childIndex + 1]->parent = me;
    me->entryCount++;

    free(returnedNode); //We have absorbed its data, get rid of the carcass...
  } else {
    //This is a leaf node.
    if(me->entryCount == BOUND_TREE_MAX_ENTRY_COUNT) {
      //Need to split this node.
      boundTree_splitNode_internal(me);
      result = me;
      //Continue as the child that should absorb the new entry
      me = me->children[me->bounds[0].bakedBound <= newEntry.bakedBound];
      childIndex = boundTree_findChildPosition(me, newEntry.bakedBound);
    }
    //Now that we have enough space here, just add the entry.
    if(childIndex < me->entryCount) { //This `if()` is needed despite `memmove(..., 0)` being a noop because we need to avoid undefined behavior when we calculate `&me->bounds[childIndex + 1]`.
      memmove(&me->bounds[childIndex + 1], &me->bounds[childIndex], (me->entryCount - childIndex)*sizeof(*me->bounds));
    }
    me->bounds[childIndex] = newEntry;
    me->entryCount++;
  }
  return result;
}

static void boundTree_add(esdmI_boundTree_t* me, int64_t bound, bool isStart, int64_t cubeIndex) {
  if(DEBUG_BOUND_TREE) boundTree_checkTree(me);
  boundTree_add_internal(me, (esdmI_boundListEntry_t){ .bakedBound = bakeBound(bound, isStart), .cubeIndex = cubeIndex});
  if(DEBUG_BOUND_TREE) boundTree_checkTree(me);
}

static int boundTree_findPositionInParent(esdmI_boundTree_t* me, esdmI_boundTree_t** out_parent) {
  eassert(out_parent);

  if(!(*out_parent = me->parent)) return -1;
  for(int i = 0; i <= me->parent->entryCount; i++) {
    if(me->parent->children[i] == me) return i;
  }
  eassert(false && "the data structure is corrupt if this code is reached");
  return -1;
}

static esdmI_boundListEntry_t* boundTree_findFirst_internal(esdmI_boundTree_t* me, int64_t bakedBound, esdmI_boundIterator_t* out_iterator) {
  //Find the first entry that is greater or equal to the given bakedBound.
  int childIndex;
  while(true) {
    childIndex = boundTree_findChildPosition(me, bakedBound - 1);
    if(!me->children[childIndex]) break;
    me = me->children[childIndex];
  }

  //Check if there is an entry with the given bakedBound in the tree.
  while(me && childIndex == me->entryCount) {
    childIndex = boundTree_findPositionInParent(me, &me); //It's pointing at our last child, the corresponding entry is in a parent.
  }
  if(me && me->bounds[childIndex].bakedBound == bakedBound) {
    out_iterator->treeIterator.node = me;
    out_iterator->treeIterator.entryPosition = childIndex;
    return &me->bounds[childIndex];
  }
    out_iterator->treeIterator.node = NULL;
    out_iterator->treeIterator.entryPosition = -1;
  return NULL;
}

static esdmI_boundListEntry_t*  boundTree_findFirst(esdmI_boundTree_t* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator) {
  if(DEBUG_BOUND_TREE) boundTree_checkTree(me);
  return boundTree_findFirst_internal(me, bakeBound(bound, isStart), out_iterator);
}

static esdmI_boundListEntry_t* boundTree_nextEntry(esdmI_boundTree_t* me, esdmI_boundIterator_t* inout_iterator) {
  eassert(inout_iterator);
  eassert(inout_iterator->treeIterator.node);
  if(DEBUG_BOUND_TREE) boundTree_checkTree(me);

  //get the info from the iterator and advance the entry position
  me = inout_iterator->treeIterator.node;
  int64_t bakedBound = me->bounds[inout_iterator->treeIterator.entryPosition].bakedBound; //remember the bound we are looking for
  int entryPosition = inout_iterator->treeIterator.entryPosition + 1;  //increment position

  //descent until we find a leaf
  while(me->children[entryPosition]) {
    me = me->children[entryPosition];
    entryPosition = 0;
  }

  //ascent until we find the entry belonging to this child position
  while(me && entryPosition == me->entryCount) {
    entryPosition = boundTree_findPositionInParent(me, &me);
  }

  //return the entry if we have found a valid one
  if(me && me->bounds[entryPosition].bakedBound == bakedBound) {
    inout_iterator->treeIterator.node = me;
    inout_iterator->treeIterator.entryPosition = entryPosition;
    return &me->bounds[entryPosition];
  }

  //there are no more entries with the given bound
  inout_iterator->treeIterator.node = NULL;
  inout_iterator->treeIterator.entryPosition = -1;
  return NULL;
}

static void boundTree_destruct(esdmI_boundTree_t* me) {
  for(int i = 0; i <= me->entryCount; i++) {
    if(me->children[i]) {
      boundTree_destruct(me->children[i]);
      free(me->children[i]);
    }
  }
}

// esdmI_boundList_t ///////////////////////////////////////////////////////////////////////////////

//Factory function to select which implementation we use.
static esdmI_boundList_t* boundList_create() {
  switch(esdmI_getConfig()->boundListImplementation) {
    default:
      ESDM_WARN("configuration parameter boundListImplementation has an illegal value, continuing with B-tree implementation");
      //fallthrough
    case BOUND_LIST_IMPLEMENTATION_BTREE: {
      esdmI_boundTree_t* result = malloc(sizeof(*result));
      boundTree_construct(result);
      return &result->super;
    }
    case BOUND_LIST_IMPLEMENTATION_ARRAY: {
      esdmI_boundArray_t* result = malloc(sizeof(*result));
      boundArray_construct(result);
      return &result->super;
    }
  }
}

// esdmI_neighbourList_t ///////////////////////////////////////////////////////////////////////////

static void neighbourList_construct(esdmI_neighbourList_t* me) {
  *me = (esdmI_neighbourList_t){
    .neighbourCount = 0,
    .allocatedCount = 8,
    .neighbourIndices = NULL
  };
  me->neighbourIndices = malloc(me->allocatedCount*sizeof(*me->neighbourIndices));
}

static void neighbourList_add(esdmI_neighbourList_t* me, int64_t index) {
  if(me->neighbourCount == me->allocatedCount) {
    me->neighbourIndices = realloc(me->neighbourIndices, (me->allocatedCount *= 2)*sizeof(*me->neighbourIndices));
  }
  eassert(me->neighbourCount < me->allocatedCount);
  me->neighbourIndices[me->neighbourCount++] = index;
}

//return the internal array of indices and their current count
static int64_t* neighbourList_getNeighbourIndices(esdmI_neighbourList_t* me, int64_t* out_neighbourCount) {
  *out_neighbourCount = me->neighbourCount;
  return me->neighbourIndices;
}

static void neighbourList_destruct(esdmI_neighbourList_t* me) {
  free(me->neighbourIndices);
}

// esdmI_hypercubeNeighbourManager_t ///////////////////////////////////////////////////////////////

esdmI_hypercubeNeighbourManager_t* esdmI_hypercubeNeighbourManager_make(int64_t dimensions) {
  esdmI_hypercubeNeighbourManager_t* result = malloc(sizeof(*result) + dimensions*sizeof(*result->boundLists));
  *result = (esdmI_hypercubeNeighbourManager_t){
    .list = {
      .cubes = NULL,
      .count = 0
    },
    .allocatedCount = 8,
    .dims = dimensions,
    .neighbourLists = NULL
  };
  result->list.cubes = malloc(result->allocatedCount*sizeof(*result->list.cubes));
  result->neighbourLists = malloc(result->allocatedCount*sizeof(*result->neighbourLists));
  for(int64_t i = 0; i < dimensions; i++) result->boundLists[i] = boundList_create();
  return result;
}

esdmI_hypercubeList_t* esdmI_hypercubeNeighbourManager_list(esdmI_hypercubeNeighbourManager_t* me) {
  return &me->list;
}

void esdmI_hypercubeNeighbourManager_pushBack(esdmI_hypercubeNeighbourManager_t* me, esdmI_hypercube_t* cube) {
  eassert(esdmI_hypercube_dimensions(cube) == me->dims);

  //assert enough space
  if(me->list.count == me->allocatedCount) {
    me->allocatedCount *= 2;
    me->list.cubes = realloc(me->list.cubes, me->allocatedCount*sizeof(*me->list.cubes));
    eassert(me->list.cubes);
    me->neighbourLists = realloc(me->neighbourLists, me->allocatedCount*sizeof(*me->neighbourLists));
    eassert(me->neighbourLists);
  }

  //give the cube an index
  int64_t cubeIndex = me->list.count++;
  eassert(cubeIndex < me->allocatedCount);
  me->list.cubes[cubeIndex] = cube;

  //find and record the neighbourhood relations of this cube
  esdmI_neighbourList_t* neighbours = &me->neighbourLists[cubeIndex];
  neighbourList_construct(neighbours);
  for(int dim = 0; dim < me->dims; dim++) {
    esdmI_boundIterator_t iterator;
    for(esdmI_boundListEntry_t* entry = boundList_findFirst(me->boundLists[dim], cube->ranges[dim].start, false, &iterator); //iterate the corresponding *end* bounds
        entry;
        entry = boundList_nextEntry(me->boundLists[dim], &iterator)
    ) {
      if(esdmI_hypercube_touches(cube, me->list.cubes[entry->cubeIndex])) {
        neighbourList_add(neighbours, entry->cubeIndex);
        neighbourList_add(&me->neighbourLists[entry->cubeIndex], cubeIndex);
      }
    }
    boundList_add(me->boundLists[dim], cube->ranges[dim].start, true, cubeIndex);

    for(esdmI_boundListEntry_t* entry = boundList_findFirst(me->boundLists[dim], cube->ranges[dim].end, true, &iterator);  //iterate the corresponding *start* bounds
        entry;
        entry = boundList_nextEntry(me->boundLists[dim], &iterator)
    ) {
      if(esdmI_hypercube_touches(cube, me->list.cubes[entry->cubeIndex])) {
        neighbourList_add(neighbours, entry->cubeIndex);
        neighbourList_add(&me->neighbourLists[entry->cubeIndex], cubeIndex);
      }
    }
    boundList_add(me->boundLists[dim], cube->ranges[dim].end, false, cubeIndex);
  }
}

int64_t* esdmI_hypercubeNeighbourManager_getNeighbours(esdmI_hypercubeNeighbourManager_t* me, int64_t index, int64_t* out_neighbourCount) {
  eassert(index < me->list.count);
  return neighbourList_getNeighbourIndices(&me->neighbourLists[index], out_neighbourCount);
}

void esdmI_hypercubeNeighbourManager_destroy(esdmI_hypercubeNeighbourManager_t* me) {
  for(int64_t i = me->list.count; i--; ) neighbourList_destruct(&me->neighbourLists[i]);
  free(me->neighbourLists);
  for(int64_t i = me->list.count; i--; ) esdmI_hypercube_destroy(me->list.cubes[i]);
  free(me->list.cubes);
  for(int64_t i = me->dims; i--; ) boundList_destruct(me->boundLists[i]), free(me->boundLists[i]);
  free(me);
}
