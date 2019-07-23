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
 * This test is a benchmark for determining the performance of esdm_dataspace_copy_data().
 */

#include <esdm.h>
#include <test/util/test_util.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define kDimSize 300

int main() {
  timer myTimer;
  start_timer(&myTimer);
  int64_t (*referenceData)[kDimSize][kDimSize] = malloc(kDimSize*sizeof(*referenceData));
  for(int64_t z = 0, value = 0; z < kDimSize; z++) {
      for(int64_t y = 0; y < kDimSize; y++) {
        for(int64_t x = 0; x < kDimSize; x++) {
          referenceData[z][y][x] = value++;
        }
      }
  }
  printf("referenceData initialization: %.3fms\n", 1000*stop_timer(myTimer));

  start_timer(&myTimer);
  int64_t (*data)[kDimSize][kDimSize] = malloc(kDimSize*sizeof(*data));
  memcpy(data, referenceData, kDimSize*sizeof(*data));
  printf("memcopy to new buffer (forcing mapping of memory pages): %.3fms\n", 1000*stop_timer(myTimer));

  start_timer(&myTimer);
  memset(data, 0, kDimSize*sizeof(*data));
  printf("memset on paged buffer: %.3fms\n", 1000*stop_timer(myTimer));

  start_timer(&myTimer);
  memcpy(data, referenceData, kDimSize*sizeof(*data));
  printf("memcopy to paged buffer: %.3fms\n", 1000*stop_timer(myTimer));

  esdm_dataspace_t *logicalSpace, *sourceSubspace, *destSubspace;
  esdm_status result = esdm_dataspace_create(3, (int64_t[3]){kDimSize, kDimSize, kDimSize}, SMD_DTYPE_INT64, &logicalSpace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){kDimSize, kDimSize, kDimSize}, (int64_t[3]){0, 0, 0}, &sourceSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){kDimSize-1, kDimSize, kDimSize}, (int64_t[3]){1, 0, 0}, &destSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(destSubspace, (int64_t[3]){kDimSize*kDimSize, kDimSize, 1});
  assert(result == ESDM_SUCCESS);
  start_timer(&myTimer);
  result = esdm_dataspace_copy_data(sourceSubspace, referenceData, destSubspace, data);
  assert(result == ESDM_SUCCESS);
  printf("copy %d block: %.3fms\n", 1, 1000*stop_timer(myTimer));
  result = esdm_dataspace_destroy(destSubspace);
  assert(result == ESDM_SUCCESS);

  start_timer(&myTimer);
  for(int64_t z = 0; z < kDimSize - 1; z++) {
      for(int64_t y = 0; y < kDimSize; y++) {
        for(int64_t x = 0; x < kDimSize; x++) {
          eassert(data[z][y][x] == referenceData[z + 1][y][x]);
        }
      }
  }
  printf("checking result: %.3fms\n", 1000*stop_timer(myTimer));

  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){kDimSize, kDimSize-1, kDimSize}, (int64_t[3]){0, 1, 0}, &destSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(destSubspace, (int64_t[3]){kDimSize*kDimSize, kDimSize, 1});
  assert(result == ESDM_SUCCESS);
  start_timer(&myTimer);
  result = esdm_dataspace_copy_data(sourceSubspace, referenceData, destSubspace, data);
  assert(result == ESDM_SUCCESS);
  printf("copy %d blocks: %.3fms\n", kDimSize, 1000*stop_timer(myTimer));
  result = esdm_dataspace_destroy(destSubspace);
  assert(result == ESDM_SUCCESS);

  start_timer(&myTimer);
  for(int64_t z = 0; z < kDimSize; z++) {
      for(int64_t y = 0; y < kDimSize - 1; y++) {
        for(int64_t x = 0; x < kDimSize; x++) {
          eassert(data[z][y][x] == referenceData[z][y + 1][x]);
        }
      }
  }
  printf("checking result: %.3fms\n", 1000*stop_timer(myTimer));

  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){kDimSize, kDimSize, kDimSize-1}, (int64_t[3]){0, 0, 1}, &destSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(destSubspace, (int64_t[3]){kDimSize*kDimSize, kDimSize, 1});
  assert(result == ESDM_SUCCESS);
  start_timer(&myTimer);
  result = esdm_dataspace_copy_data(sourceSubspace, referenceData, destSubspace, data);
  assert(result == ESDM_SUCCESS);
  printf("copy %d blocks: %.3fms\n", kDimSize*kDimSize, 1000*stop_timer(myTimer));
  result = esdm_dataspace_destroy(destSubspace);
  assert(result == ESDM_SUCCESS);

  start_timer(&myTimer);
  for(int64_t z = 0; z < kDimSize; z++) {
      for(int64_t y = 0; y < kDimSize; y++) {
        for(int64_t x = 0; x < kDimSize - 1; x++) {
          eassert(data[z][y][x] == referenceData[z][y][x + 1]);
        }
      }
  }
  printf("checking result: %.3fms\n", 1000*stop_timer(myTimer));

  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){kDimSize, kDimSize, kDimSize}, (int64_t[3]){0, 0, 0}, &destSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(destSubspace, (int64_t[3]){kDimSize*kDimSize, 1, kDimSize});
  assert(result == ESDM_SUCCESS);
  start_timer(&myTimer);
  result = esdm_dataspace_copy_data(sourceSubspace, referenceData, destSubspace, data);
  assert(result == ESDM_SUCCESS);
  printf("transpose two inner dimensions (copy %d blocks): %.3fms\n", kDimSize*kDimSize*kDimSize, 1000*stop_timer(myTimer));
  result = esdm_dataspace_destroy(destSubspace);
  assert(result == ESDM_SUCCESS);

  start_timer(&myTimer);
  for(int64_t z = 0; z < kDimSize; z++) {
      for(int64_t y = 0; y < kDimSize; y++) {
        for(int64_t x = 0; x < kDimSize; x++) {
          eassert(data[z][x][y] == referenceData[z][y][x]);
        }
      }
  }
  printf("checking result: %.3fms\n", 1000*stop_timer(myTimer));

  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){kDimSize, kDimSize, kDimSize}, (int64_t[3]){0, 0, 0}, &destSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(destSubspace, (int64_t[3]){1, kDimSize, kDimSize*kDimSize}); //expected to be brutally inefficient due to really bad cache usage
  assert(result == ESDM_SUCCESS);
  start_timer(&myTimer);
  result = esdm_dataspace_copy_data(sourceSubspace, referenceData, destSubspace, data);
  assert(result == ESDM_SUCCESS);
  printf("transforming to FORTRAN order (copy %d blocks): %.3fms\n", kDimSize*kDimSize*kDimSize, 1000*stop_timer(myTimer));
  result = esdm_dataspace_destroy(destSubspace);
  assert(result == ESDM_SUCCESS);

  start_timer(&myTimer);
  for(int64_t z = 0; z < kDimSize; z++) {
      for(int64_t y = 0; y < kDimSize; y++) {
        for(int64_t x = 0; x < kDimSize; x++) {
          eassert(data[x][y][z] == referenceData[z][y][x]);
        }
      }
  }
  printf("checking result: %.3fms\n", 1000*stop_timer(myTimer));

  result = esdm_dataspace_destroy(logicalSpace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_destroy(sourceSubspace);
  assert(result == ESDM_SUCCESS);

  printf("\nOK\n");
}
