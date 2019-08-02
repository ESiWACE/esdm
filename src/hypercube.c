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
    .count = 0,
    .allocatedCount = 8
  };
  me->cubes = malloc(me->allocatedCount*sizeof(*me->cubes));
  eassert(me->cubes);
}

void esdmI_hypercubeSet_add(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube) {
  eassert(me);
  eassert(cube);

  //grow the buffer if necessary
  if(me->count == me->allocatedCount) {
    me->allocatedCount <<= 1;
    me->cubes = realloc(me->cubes, me->allocatedCount*sizeof(*me->cubes));
    eassert(me->cubes);
  }
  eassert(me->allocatedCount > me->count);

  //add the new element to the set
  me->cubes[me->count++] = esdmI_hypercube_makeCopy(cube);
}

void esdmI_hypercubeSet_subtract(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* subtrahend) {
  eassert(me);
  eassert(subtrahend);

  int64_t dimensions = subtrahend->dims;

  for(int64_t i = me->count; i--; ) {  //iterate backwards so that we can freely modify the tail of the array
    //check whether we need to modify this hypercube at all
    esdmI_hypercube_t* minuend = me->cubes[i];
    eassert(minuend->dims == dimensions);
    if(!esdmI_hypercube_doesIntersect(minuend, subtrahend)) continue; //fast path: no intersection, the minuend remains in the set unmodified

    //we need to modify this cube, so take it out of the set
    me->cubes[i] = me->cubes[--me->count];
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

bool esdmI_hypercubeSet_doesIntersect(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube) {
  for(int64_t i = 0; i < me->count; i++) {
    if(esdmI_hypercube_doesIntersect(me->cubes[i], cube)) return true;
  }
  return false;
}

void esdmI_hypercubeSet_print(esdmI_hypercubeSet_t* me, FILE* stream) {
  for(int64_t i = 0; i < me->count; i++) {
    esdmI_hypercube_print(me->cubes[i], stream);
  }
}

void esdmI_hypercubeSet_destruct(esdmI_hypercubeSet_t* me) {
  for(int64_t i = 0; i < me->count; i++) {
    esdmI_hypercube_destroy(me->cubes[i]);
  }
  free(me->cubes);
  memset(me, 0, sizeof(*me));
}

void esdmI_hypercubeSet_destroy(esdmI_hypercubeSet_t* me) {
  esdmI_hypercubeSet_destruct(me);
  free(me);
}
