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

/*
 * This test is a proof of concept for arbitrary hypercube memory mappings.
 */

#include <test/util/test_util.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dataspace {
  uint64_t dimensions;
  int64_t *offset, *stride;
  uint64_t *size;
} dataspace;

void* memdup(void* data, size_t size) {
  void* result = malloc(size);
  memcpy(result, data, size);
  return result;
}

dataspace* dataspace_create(uint64_t dimensions, int64_t* offset, uint64_t* size, int64_t* stride) {
  dataspace* result = malloc(sizeof(*result));
  *result = (dataspace){
    .dimensions = dimensions,
    .offset = memdup(offset, dimensions*sizeof(*offset)),
    .size = memdup(size, dimensions*sizeof(*size)),
    .stride = memdup(stride, dimensions*sizeof(*stride))
  };
  return result;
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

static void copyData(void* dest, void* source, uint64_t size) {
  printf("copying %"PRIu64" bytes from 0x%016"PRIx64" to 0x%016"PRIx64"\n", size, (uint64_t)source, (uint64_t)dest);
  memcpy(dest, source, size);
}

void dataspace_copy_data(dataspace* sourceSpace, int64_t *sourceData, dataspace* destSpace, int64_t *destData) {
  eassert(sourceSpace->dimensions == destSpace->dimensions);

  uint64_t dimensions = sourceSpace->dimensions;

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
    if(overlapX0 > overlapX1) return; //overlap is empty => nothing to do
  }

  //determine how much data we can move with memcpy() at a time
  int64_t dataPointerOffset = 0, memcpySize = 1;  //both are counts of fundamental elements, not bytes
  bool memcpyDims[dimensions];
  memset(memcpyDims, 0, sizeof(memcpyDims));
  while(true) {
    //search for a dimension with a stride that matches our current memcpySize
    int64_t curDim;
    for(curDim = dimensions; curDim--; ) {
      if(sourceSpace->stride[curDim] == destSpace->stride[curDim]) {
        if(abs_int64(sourceSpace->stride[curDim]) == memcpySize) break;
      }
    }
    if(curDim >= 0) {
      //found a fitting dimension, update our parameters
      memcpyDims[curDim] = true;  //remember not to loop over this dimension when copying the data
      if(sourceSpace->stride[curDim] < 0) dataPointerOffset += memcpySize*(overlapSize[curDim] - 1); //When the stride is negative, the first byte belongs to the last slice of this dimension, not the first one. Remember that.
      memcpySize *= overlapSize[curDim];
      if(overlapSize[curDim] != sourceSpace->size[curDim] || overlapSize[curDim] != destSpace->size[curDim]) break; //cannot fuse other dimensions in a memcpy() call if this dimensions does not have a perfect match between the dataspaces
    } else break; //didn't find another suitable dimension for fusing memcpy() calls
  }

  //copy the data
  int64_t curPoint[dimensions];
  memcpy(curPoint, overlapOffset, sizeof(curPoint));
  while(true) {
    //compute the parameters for the memcpy() at hand
    int64_t sourceIndex = 0, destIndex = 0;
    for(int64_t i = 0; i < dimensions; i++) {
      sourceIndex += (curPoint[i] - sourceSpace->offset[i])*sourceSpace->stride[i];
      destIndex += (curPoint[i] - destSpace->offset[i])*destSpace->stride[i];
    }

    //move the data
    copyData(&destData[destIndex - dataPointerOffset], &sourceData[sourceIndex - dataPointerOffset], memcpySize*sizeof(*destData));

    //advance to the next point
    int64_t curDim;
    for(curDim = dimensions; curDim--; ) {
      if(!memcpyDims[curDim]) {
        if(++(curPoint[curDim]) < overlapOffset[curDim] + overlapSize[curDim]) break;
        curPoint[curDim] = overlapOffset[curDim];
      }
    }
    if(curDim < 0) break;
  }
}


void dataspace_destroy(dataspace* this) {
  free(this->offset);
  free(this->size);
  free(this->stride);
  free(this);
}

int main() {
  dataspace* sourceSpace = dataspace_create(3, (int64_t[3]){6, 0, -7}, (uint64_t[3]){2, 3, 5}, (int64_t[3]){-3, 1, 6});
  int64_t sourceData[30] = {
    0x709, 0x719, 0x729,
    0x609, 0x619, 0x629,

    0x70a, 0x71a, 0x72a,
    0x60a, 0x61a, 0x62a,

    0x70b, 0x71b, 0x72b,
    0x60b, 0x61b, 0x62b,

    0x70c, 0x71c, 0x72c,
    0x60c, 0x61c, 0x62c,

    0x70d, 0x71d, 0x72d,
    0x60d, 0x61d, 0x62d
  }, *firstSourceByte = &sourceData[3];

  dataspace* destSpace = dataspace_create(3, (int64_t[3]){5, 0, -6}, (uint64_t[3]){3, 3, 3}, (int64_t[3]){-3, 1, -9});
  int64_t destData[27] = {0}, expectedData[27] = {
    0x70c, 0x71c, 0x72c,
    0x60c, 0x61c, 0x62c,
    0x000, 0x000, 0x000,

    0x70b, 0x71b, 0x72b,
    0x60b, 0x61b, 0x62b,
    0x000, 0x000, 0x000,

    0x70a, 0x71a, 0x72a,
    0x60a, 0x61a, 0x62a,
    0x000, 0x000, 0x000,
  }, *firstDestByte = &destData[6 + 2*9];

  dataspace_copy_data(sourceSpace, firstSourceByte, destSpace, firstDestByte);

  for(int64_t i = 0; i < 27; i++) eassert(destData[i] == expectedData[i]);

  printf("\nOK\n");
}
