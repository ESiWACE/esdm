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
 * @brief This file implements the hypercube related classes, `esdmI_range_t`, `esdmI_hypercube_t`, and `esdmI_hypercubeSet_t`.
 */

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <esdm-internal.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

//Generate symbols for the inline functions.
esdmI_range_t esdmI_range_intersection(esdmI_range_t a, esdmI_range_t b);
bool esdmI_range_isEmpty(esdmI_range_t range);
int64_t esdmI_range_size(esdmI_range_t range);

void esdmI_range_print(esdmI_range_t range, FILE* stream) {
  if(esdmI_range_isEmpty(range)) {
    fprintf(stream, "[empty]");
  } else if(range.start + 1 == range.end) {
    fprintf(stream, "[%"PRId64"]", range.start);
  } else if(range.start + 2 == range.end) {
    fprintf(stream, "[%"PRId64", %"PRId64"]", range.start, range.end - 1);
  } else {
    fprintf(stream, "[%"PRId64", ..., %"PRId64"]", range.start, range.end - 1);
  }
}

esdmI_hypercube_t* esdmI_hypercube_makeDefault(int64_t dimensions) {
  eassert(dimensions >= 0);

  esdmI_hypercube_t* result = malloc(sizeof(*result) + dimensions*sizeof(*result->ranges));
  result->dims = dimensions;
  for(int64_t i = 0; i < dimensions; i++) {
    result->ranges[i] = (esdmI_range_t){
      .start = 0,
      .end = 0
    };
  }
  return result;
}

esdmI_hypercube_t* esdmI_hypercube_make(int64_t dimensions, int64_t* offset, int64_t* size) {
  eassert(offset);
  eassert(size);

  esdmI_hypercube_t* result = malloc(sizeof(*result) + dimensions*sizeof(*result->ranges));
  result->dims = dimensions;
  for(int64_t i = 0; i < dimensions; i++) {
    result->ranges[i] = (esdmI_range_t){
      .start = offset[i],
      .end = offset[i] + size[i]
    };
  }
  return result;
}

esdmI_hypercube_t* esdmI_hypercube_makeCopy(esdmI_hypercube_t* original) {
  int64_t size = sizeof(*original) + original->dims*sizeof(*original->ranges);
  esdmI_hypercube_t* result = malloc(size);
  memcpy(result, original, size);
  return result;
}

esdmI_hypercube_t* esdmI_hypercube_makeIntersection(esdmI_hypercube_t* a, esdmI_hypercube_t* b) {
  eassert(a);
  eassert(b);
  eassert(a->dims == b->dims);
  int64_t dimensions = a->dims;

  esdmI_hypercube_t* result = malloc(sizeof(*result) + dimensions*sizeof(*result->ranges));
  result->dims = dimensions;
  for(int64_t i = 0; i < dimensions; i++) {
    result->ranges[i] = esdmI_range_intersection(a->ranges[i], b->ranges[i]);
    if(esdmI_range_isEmpty(result->ranges[i])) {
      free(result);
      return NULL;
    }
  }
  return result;
}

bool esdmI_hypercube_isEmpty(esdmI_hypercube_t* me) {
  eassert(me);

  for(int64_t i = 0; i < me->dims; i++) {
    if(esdmI_range_isEmpty(me->ranges[i])) return true;
  }
  return false;
}

bool esdmI_hypercube_doesIntersect(esdmI_hypercube_t* a, esdmI_hypercube_t* b) {
  eassert(a);
  eassert(b);
  eassert(a->dims == b->dims);
  int64_t dimensions = a->dims;

  for(int64_t i = 0; i < dimensions; i++) {
    if(esdmI_range_isEmpty(esdmI_range_intersection(a->ranges[i], b->ranges[i]))) return false;
  }
  return true;
}

int64_t esdmI_hypercube_dimensions(esdmI_hypercube_t* cube);  //instantiate inline function

void esdmI_hypercube_getOffsetAndSize(esdmI_hypercube_t* cube, int64_t* out_offset, int64_t* out_size) {
  eassert(out_offset);
  eassert(out_size);

  for(int64_t i = 0; i < cube->dims; i++) {
    out_offset[i] = cube->ranges[i].start;
    out_size[i] = esdmI_range_size(cube->ranges[i]);
  }
}

int64_t esdmI_hypercube_size(esdmI_hypercube_t* cube) {
  eassert(cube);
  eassert(cube->dims > 0);

  int64_t result = 1;
  for(int64_t i = 0; i < cube->dims; i++) {
    result *= esdmI_range_size(cube->ranges[i]);
  }
  return result;
}

void esdmI_hypercube_print(esdmI_hypercube_t* cube, FILE* stream) {
  fprintf(stream, "(");
  for(int64_t i = 0; i < cube->dims; i++) {
    if(i) fprintf(stream, ", ");
    esdmI_range_print(cube->ranges[i], stream);
  }
  fprintf(stream, ")\n");
}

void esdmI_hypercube_destroy(esdmI_hypercube_t* cube) {
  free(cube);
}

esdmI_hypercubeSet_t* esdmI_hypercubeSet_make() {
  esdmI_hypercubeSet_t* me = malloc(sizeof(*me));
  esdmI_hypercubeSet_construct(me);
  return me;
}

void esdmI_hypercubeSet_construct(esdmI_hypercubeSet_t* me) {
  *me = (esdmI_hypercubeSet_t){
    .list = {.cubes = NULL, .count = 0},
    .allocatedCount = 8
  };
  me->list.cubes = malloc(me->allocatedCount*sizeof(*me->list.cubes));
  eassert(me->list.cubes);
}

esdmI_hypercubeList_t* esdmI_hypercubeSet_list(esdmI_hypercubeSet_t* me) { return &me->list; }

bool esdmI_hypercubeSet_isEmpty(esdmI_hypercubeSet_t* me) {
  for(int64_t i = me->list.count; i--; ) { //iterate backwards since we may remove hypercubes from the set
    if(esdmI_hypercube_isEmpty(me->list.cubes[i])) {
      //remove the empty cube from the set
      esdmI_hypercube_destroy(me->list.cubes[i]);
      me->list.cubes[i] = me->list.cubes[--me->list.count];
    } else {
      return false; //found one non-empty hypercube, that's enough
    }
  }

  eassert(me->list.count == 0);
  return true;
}

int64_t esdmI_hypercubeSet_count(esdmI_hypercubeSet_t* me) {
  return me->list.count;
}

void esdmI_hypercubeSet_add(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube) {
  eassert(me);
  eassert(cube);

  //grow the buffer if necessary
  if(me->list.count == me->allocatedCount) {
    me->allocatedCount <<= 1;
    me->list.cubes = realloc(me->list.cubes, me->allocatedCount*sizeof(*me->list.cubes));
    eassert(me->list.cubes);
  }
  eassert(me->allocatedCount > me->list.count);

  //add the new element to the set
  me->list.cubes[me->list.count++] = esdmI_hypercube_makeCopy(cube);
}

void esdmI_hypercubeSet_subtract(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* subtrahend) {
  eassert(me);
  eassert(subtrahend);

  int64_t dimensions = subtrahend->dims;

  for(int64_t i = me->list.count; i--; ) {  //iterate backwards so that we can freely modify the tail of the array
    //check whether we need to modify this hypercube at all
    esdmI_hypercube_t* minuend = me->list.cubes[i];
    eassert(minuend->dims == dimensions);
    if(!esdmI_hypercube_doesIntersect(minuend, subtrahend)) continue; //fast path: no intersection, the minuend remains in the set unmodified

    //we need to modify this cube, so take it out of the set
    me->list.cubes[i] = me->list.cubes[--me->list.count];
    eassert(minuend->dims == dimensions);

    //Determine how much of that hypercube we need to readd to the set.
    //For this, we consider one dimension after the other, splitting the minuend into three parts:
    //The part before the start of the subtrahend, the part that intersects with the subtrahend, and the part after the end of the subtrahend.
    //The first and last parts are then readded as a single hypercube to the set (if they are not empty),
    //while the splitting of the intersecting part is continued for the other dimensions.
    for(int64_t curDim = 0; curDim < dimensions; curDim++) {
      //take the part of the minuend that is before the start of the subtrahend in this dimension
      if(minuend->ranges[curDim].start < subtrahend->ranges[curDim].start) {
        esdmI_hypercube_t* copy = esdmI_hypercube_makeCopy(minuend);
        if(copy->ranges[curDim].end > subtrahend->ranges[curDim].start) {
          copy->ranges[curDim].end = subtrahend->ranges[curDim].start;
        }
        esdmI_hypercubeSet_add(me, copy);
      }
      //take the part of the minuend that is behind the end of the subtrahend in this dimension
      if(minuend->ranges[curDim].end > subtrahend->ranges[curDim].end) {
        esdmI_hypercube_t* copy = esdmI_hypercube_makeCopy(minuend);
        if(copy->ranges[curDim].start < subtrahend->ranges[curDim].end) {
          copy->ranges[curDim].start = subtrahend->ranges[curDim].end;
        }
        esdmI_hypercubeSet_add(me, copy);
      }
      //Reduce the minuend in this dimension to the intersecting part.
      //This ensures that further splits along other dimensions won't intersect with the two parts we readded above.
      //The resulting range cannot be empty because we have already checked above that an intersection actually exists.
      minuend->ranges[curDim] = esdmI_range_intersection(minuend->ranges[curDim], subtrahend->ranges[curDim]);
    }
  }
}

bool esdmI_hypercubeList_doesIntersect(esdmI_hypercubeList_t* list, esdmI_hypercube_t* cube) {
  for(int64_t i = 0; i < list->count; i++) {
    if(esdmI_hypercube_doesIntersect(list->cubes[i], cube)) return true;
  }
  return false;
}

//Fill the given 2D array with the intersections of all the hypercubes within the set.
//`count` must equal `me->list.count`, it must be passed redundantly to be usable as an array dimension for `out_intersectionMatrix` and `out_intersectionSizes`.
//`out_intersectionMatrix` and `out_intersectionSizes` return a pointers to 2D arrays, call `destroyIntersectionMatrix()` to destruct/deallocate them.
//
//The self-intersection entries of the matrix are filled with NULL pointers, but the respective size is set to that of the self-intersecting cube.
#define makeIntersectionMatrix(list, out_intersectionMatrix, out_intersectionSizes) do {\
  esdmI_hypercubeList_t* l = list;\
  makeIntersectionMatrix_internal(l, l->count, out_intersectionMatrix, out_intersectionSizes);\
} while(false)
static void makeIntersectionMatrix_internal(esdmI_hypercubeList_t* list, int64_t count, esdmI_hypercube_t* (**out_intersectionMatrix)[count], int64_t (**out_intersectionSizes)[count]) {
  eassert(list);
  eassert(list->count == count);
  eassert(out_intersectionMatrix);
  eassert(out_intersectionSizes);

  esdmI_hypercube_t* (*intersectionMatrix)[count] = malloc(count*sizeof(*intersectionMatrix));
  int64_t (*intersectionSizes)[count] = malloc(count*sizeof(*intersectionSizes));

  for(int64_t i = 0; i < count; i++) {
    intersectionMatrix[i][i] = NULL;  //The algorithms relying on the intersection matrix work better when the self intersection is NULL.
    intersectionSizes[i][i] = esdmI_hypercube_size(list->cubes[i]); //However, they need to know the size of the self intersection. I know, this does not look sensible, but it's what is actually needed.
    for(int64_t j = i+1; j < count; j++) {
      esdmI_hypercube_t* curIntersection = esdmI_hypercube_makeIntersection(list->cubes[i], list->cubes[j]);
      intersectionMatrix[i][j] = intersectionMatrix[j][i] = curIntersection;
      intersectionSizes[i][j] = intersectionSizes[j][i] = curIntersection ? esdmI_hypercube_size(curIntersection) : 0;
    }
  }

  *out_intersectionMatrix = intersectionMatrix;
  *out_intersectionSizes = intersectionSizes;
}

static void destroyIntersectionMatrix(int64_t count, esdmI_hypercube_t* (*intersectionMatrix)[count], int64_t (*intersectionSizes)[count]) {
  for(int64_t i = 0; i < count; i++) {
    for(int64_t j = i+1; j < count; j++) {  //only destroy the hypercubes above the diagonal; the cubes on the diagonal are not owned by the matrix, and the pointers balow the diagonal alias the pointers above the diagonal
      if(intersectionMatrix[i][j]) esdmI_hypercube_destroy(intersectionMatrix[i][j]);
    }
  }
  free(intersectionMatrix);
  free(intersectionSizes);
}

//Check whether the `coveringCubes` fully cover the `coveredCube`.
//Some or all of the pointers in `coveringCubes` may be NULL.
static bool hypercubeIsFullyCovered(esdmI_hypercube_t* coveredCube, int64_t coveringCubesCount, esdmI_hypercube_t** coveringCubes) {
  eassert(coveredCube);
  eassert(coveringCubesCount >= 0);
  eassert(coveringCubes);

  esdmI_hypercubeSet_t restSet;
  esdmI_hypercubeSet_construct(&restSet);
  esdmI_hypercubeSet_add(&restSet, coveredCube);

  for(int64_t i = 0; i < coveringCubesCount; i++) {
    if(coveringCubes[i]) {
      esdmI_hypercubeSet_subtract(&restSet, coveringCubes[i]);
      if(!restSet.list.count) break; //fast exit once the restSet becomes empty, this line is not necessary, but it might speed up some cases a bit
    }
  }
  bool result = !restSet.list.count;
  esdmI_hypercubeSet_destruct(&restSet);
  return result;
}

//Probabilistically find a single minimal subset of cubes that covers the entire space.
#define findMinimalSubset(list, requiredCubes, intersectionMatrix, out_selectedCubes) do {\
  esdmI_hypercubeList_t* l = list;\
  findMinimalSubset_internal(l->count, l->cubes, requiredCubes, intersectionMatrix, out_selectedCubes);\
} while(false)
static void findMinimalSubset_internal(int64_t count, esdmI_hypercube_t** cubes, uint8_t* requiredCubes, esdmI_hypercube_t* (*intersectionMatrix)[count], uint8_t* out_selectedCubes) {
  eassert(cubes);
  eassert(requiredCubes);
  eassert(intersectionMatrix);
  eassert(out_selectedCubes);

  memset(out_selectedCubes, true, count*sizeof(*out_selectedCubes));
  uint8_t* checkedCubes = ea_memdup(requiredCubes, count*sizeof(*checkedCubes));
  esdmI_hypercube_t* (*reducedMatrix)[count] = ea_memdup(intersectionMatrix, count*sizeof(*reducedMatrix));

  //How many cubes do we need to check?
  uint64_t uncheckedCubeCount = 0;
  for(int64_t i = 0; i < count; i++) uncheckedCubeCount += !checkedCubes[i];

  for(; uncheckedCubeCount; uncheckedCubeCount--) {
    //randomly select a yet unchecked cube for checking
    static unsigned int seed = 42;
    uint64_t randomValue = rand_r(&seed);
    uint64_t cubeSelector = randomValue*uncheckedCubeCount/((uint64_t)RAND_MAX + 1);  //this is an index into the cubes for which the checkedCubes[] bit is not set
    int64_t checkIndex = 0;
    for(; cubeSelector >= 0; checkIndex++) if(!checkedCubes[checkIndex]) cubeSelector--;  //turn the cubeSelector into a real index

    //determine whether we can drop this cube
    out_selectedCubes[checkIndex] = !hypercubeIsFullyCovered(cubes[checkIndex], count, reducedMatrix[checkIndex]);
    if(!out_selectedCubes[checkIndex]) {
      for(int64_t j = 0; j < count; j++) reducedMatrix[checkIndex][j] = reducedMatrix[j][checkIndex] = NULL;  //remove the dropped cube from the intersection matrix
    }
    checkedCubes[checkIndex] = true;
  }

  //cleanup
  free(checkedCubes);
  free(reducedMatrix);
}

//This function returns a number of minimal subsets of the cubes contained within the hypercubeSet.
//The selection of the subsets is probabilistic as any complete algorithm I could think of would have had exponential complexity.
//The probabilistic solution allows the caller to specify how much effort should be spent in finding minimal subsets.
//With the probabilistic solution, the function creates as many subsets as indicated by `*inout_setCount`,
//and returns the number of actually found subsets in the same memory location.
//The returned value may be lower than the given value because a subset may be found several times.
//
//The probabilistic algorithm is written in such a way that it will find small minimal subsets more easily than subsets that contain more cubes.
//
//The complexity of the algorithm is `O(*inout_setCount * list->count^2)`.
//
//`*out_subsets` returns an array of `*inout_setCount` `esdmI_hypercubeList_t` objects.
//It is the callers' responsibility to free the `*out_subsets` array.
void esdmI_hypercubeList_nonredundantSubsets(esdmI_hypercubeList_t* list, int64_t* inout_setCount, esdmI_hypercubeList_t** out_subsets) {
  eassert(list);
  eassert(inout_setCount);
  eassert(out_subsets);

  //Determine which hypercubes in the set are needed unconditionally.
  esdmI_hypercube_t* (*intersectionMatrix)[list->count];
  int64_t (*intersectionSizes)[list->count];
  makeIntersectionMatrix(list, &intersectionMatrix, &intersectionSizes);
  uint8_t* requiredCubes = malloc(list->count*sizeof(*requiredCubes));
  for(int64_t i = 0; i < list->count; i++) {
    int64_t coverage = 0;
    for(int64_t j = 0; j < list->count; j++) coverage += intersectionSizes[i][j];
    if(coverage < 2*intersectionSizes[i][i]) {
      //the coverage includes the size of the hypercube itself, and if it's not at least twice as big, some parts of the hypercube cannot be covered by other cubes
      requiredCubes[i] = true;
    } else {
      //Ok, coverage check is positive. Now check whether all points inside this cube are actually covered by other cubes or not.
      requiredCubes[i] = hypercubeIsFullyCovered(list->cubes[i], list->count, intersectionMatrix[i]);
    }
  }

  //Probabilistically create a number of minimal sets.
  int64_t setCount = 0;
  uint8_t (*sets)[list->count] = malloc(*inout_setCount*sizeof(*sets)); //these are used as booleans, 1 means that the respective cube is used in the subset
  for(; setCount < *inout_setCount; setCount++) {
    findMinimalSubset(list, requiredCubes, intersectionMatrix, sets[setCount]);
    //FIXME: Remove duplicate sets.
  }

  destroyIntersectionMatrix(list->count, intersectionMatrix, intersectionSizes);

  //transform the data in `sets` to `out_subsets`
  int64_t totalCubes = 0;
  for(int64_t i = 0; i < setCount; i++) {
    for(int64_t j = 0; j < list->count; j++) {
      totalCubes += sets[i][j];
    }
  }
  esdmI_hypercube_t** pointers;
  esdmI_hypercubeList_t* subsets = malloc(setCount*sizeof(*subsets) + totalCubes*sizeof(*pointers)); //allocate the memory for both `pointers` and `subsets` in one go
  pointers = (esdmI_hypercube_t**)(subsets + setCount);  //place the pointer array after the hypercube list array
  for(int64_t i = 0; i < setCount; i++) {
    subsets[i] = (esdmI_hypercubeList_t){
      .cubes = pointers,
      .count = 0
    };
    for(int64_t j = 0; j < list->count; j++) {
      if(sets[i][j]) {
        *(pointers++) = list->cubes[j];
        subsets[i].count++;
      }
    }
  }

  *inout_setCount = setCount;
  *out_subsets = subsets;
}

bool esdmI_hypercubeList_doesCoverFully(esdmI_hypercubeList_t* list, esdmI_hypercube_t* cube) {
  //make a list of the cubes in the set that actually intersect with cube, this avoids unnecessary splitting and reduces the total workload of this algorithm
  esdmI_hypercube_t** intersectingCubes = malloc(list->count*sizeof(*intersectingCubes));
  int64_t intersectingCubesCount = 0;
  for(int64_t i = 0; i < list->count; i++) {
    if(esdmI_hypercube_doesIntersect(list->cubes[i], cube)) {
      intersectingCubes[intersectingCubesCount++] = list->cubes[i];
    }
  }

  //perform the check
  bool result = hypercubeIsFullyCovered(cube, intersectingCubesCount, intersectingCubes);

  //cleanup
  free(intersectingCubes);
  return result;
}

void esdmI_hypercubeList_print(esdmI_hypercubeList_t* list, FILE* stream) {
  for(int64_t i = 0; i < list->count; i++) {
    esdmI_hypercube_print(list->cubes[i], stream);
  }
}

void esdmI_hypercubeSet_destruct(esdmI_hypercubeSet_t* me) {
  for(int64_t i = 0; i < me->list.count; i++) {
    esdmI_hypercube_destroy(me->list.cubes[i]);
  }
  free(me->list.cubes);
  memset(me, 0, sizeof(*me));
}

void esdmI_hypercubeSet_destroy(esdmI_hypercubeSet_t* me) {
  esdmI_hypercubeSet_destruct(me);
  free(me);
}
