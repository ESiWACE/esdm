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

// esdmI_boundList_t ///////////////////////////////////////////////////////////////////////////////

static void boundList_construct(esdmI_boundList_t* me) {
  *me = (esdmI_boundList_t){
    .entries = NULL,
    .count = 0,
    .allocatedCount = 16
  };
  me->entries = malloc(me->allocatedCount*sizeof(*me->entries));
  eassert(me->entries);
}

//This is a no-fail lookup that always returns either the first entry with the given bound, or the place where such an entry should be inserted into the list.
//As such, it may return a pointer past the end of the array if the bound is larger than all existing entries.
static esdmI_boundListEntry_t* boundList_lookup(esdmI_boundList_t* me, int64_t bakedBound) {
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

static void boundList_add(esdmI_boundList_t* me, int64_t bound, bool isStart, int64_t cubeIndex) {
  //make sure we have enough space
  if(me->count == me->allocatedCount) {
    me->allocatedCount *= 2;
    me->entries = realloc(me->entries, me->allocatedCount*sizeof(*me->entries));
    eassert(me->entries);
  }
  eassert(me->allocatedCount > me->count);

  int64_t bakedBound = bakeBound(bound, isStart);
  esdmI_boundListEntry_t* iterator = boundList_lookup(me, bakedBound); //get the place where to insert the entry …
  memmove(iterator + 1, iterator, (me->entries + me->count++ - iterator)*sizeof(*iterator));  //… shift everything after that …
  *iterator = (esdmI_boundListEntry_t){ //… and insert the new entry
    .bakedBound = bakedBound,
    .cubeIndex = cubeIndex
  };
}

//find the first entry with the given bound, or return NULL if no such entry exists
static esdmI_boundListEntry_t* boundList_findFirst(esdmI_boundList_t* me, int64_t bound, bool isStart, esdmI_boundListEntry_t** out_iterator) {
  eassert(out_iterator);
  int64_t bakedBound = bakeBound(bound, isStart);
  esdmI_boundListEntry_t* result = boundList_lookup(me, bakedBound);
  if(result - me->entries >= me->count) return NULL;
  if(result->bakedBound != bakedBound) return NULL;
  return *out_iterator = result;
}

//returns the next entry with the same bound, or NULL
static esdmI_boundListEntry_t* boundList_nextEntry(esdmI_boundList_t* me, esdmI_boundListEntry_t** inout_iterator) {
  eassert(inout_iterator);
  int64_t nextIndex = *inout_iterator - me->entries + 1;
  if(nextIndex >= me->count) return NULL;
  if(me->entries[nextIndex].bakedBound != (*inout_iterator)->bakedBound) return NULL;
  return *inout_iterator = &me->entries[nextIndex];
}

static void boundList_destruct(esdmI_boundList_t* me) {
  free(me->entries);
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
  for(int64_t i = 0; i < dimensions; i++) boundList_construct(&result->boundLists[i]);
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
    esdmI_boundListEntry_t* iterator;
    for(esdmI_boundListEntry_t* entry = boundList_findFirst(&me->boundLists[dim], cube->ranges[dim].start, false, &iterator); //iterate the corresponding *end* bounds
        entry;
        entry = boundList_nextEntry(&me->boundLists[dim], &iterator)
    ) {
      if(esdmI_hypercube_touches(cube, me->list.cubes[entry->cubeIndex])) {
        neighbourList_add(neighbours, entry->cubeIndex);
        neighbourList_add(&me->neighbourLists[entry->cubeIndex], cubeIndex);
      }
    }
    boundList_add(&me->boundLists[dim], cube->ranges[dim].start, true, cubeIndex);

    for(esdmI_boundListEntry_t* entry = boundList_findFirst(&me->boundLists[dim], cube->ranges[dim].end, true, &iterator);  //iterate the corresponding *start* bounds
        entry;
        entry = boundList_nextEntry(&me->boundLists[dim], &iterator)
    ) {
      if(esdmI_hypercube_touches(cube, me->list.cubes[entry->cubeIndex])) {
        neighbourList_add(neighbours, entry->cubeIndex);
        neighbourList_add(&me->neighbourLists[entry->cubeIndex], cubeIndex);
      }
    }
    boundList_add(&me->boundLists[dim], cube->ranges[dim].end, false, cubeIndex);
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
  for(int64_t i = me->dims; i--; ) boundList_destruct(&me->boundLists[i]);
  free(me);
}
