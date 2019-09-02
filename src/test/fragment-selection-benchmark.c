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
 * This test uses the ESDM high-level API to actually write a contiuous ND subset of a data set
 */


#include <esdm.h>
#include <stdio.h>
#include <stdlib.h>

#include <esdm-internal.h>

const int64_t k_dims = 3;
const int64_t k_edgeLength = 100;

int64_t totalElements(int64_t dims, int64_t* dimSizes) {
  int64_t result = 1;
  while(dims--) result *= dimSizes[dims];
  return result;
}

int64_t totalSize(int64_t dims, int64_t* dimSizes) {
  return sizeof(uint64_t)*totalElements(dims, dimSizes);
}

void fakeData(int64_t dims, int64_t* dimSizes, uint64_t** out_data) {
  *out_data = malloc(totalSize(dims, dimSizes));
  for(int64_t i = totalElements(dims, dimSizes); i--; ) (*out_data)[i] = i;
}

void clearData(int64_t dims, int64_t* dimSizes, uint64_t* data) {
  memset(data, 0, totalSize(dims, dimSizes));
}

bool dataIsCorrect(int64_t dims, int64_t* dimSizes, uint64_t* data) {
  static uint64_t* referenceData = NULL;
  if(!referenceData) fakeData(dims, dimSizes, &referenceData);
  return !memcmp(data, referenceData, totalSize(dims, dimSizes));
}

void writeFragment(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t dims, int64_t* offset, int64_t* size, uint64_t* data) {
  esdm_dataspace_t* subspace;
  int ret = esdm_dataspace_subspace(dataspace, dims, size, offset, &subspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataspace_copyDatalayout(subspace, dataspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_write(dataset, (char*)data + esdm_dataspace_elementOffset(dataspace, offset), subspace);
  eassert(ret == ESDM_SUCCESS);
}

void writeSliced(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t dims, int64_t* dimSizes, uint64_t* data, int64_t sliceDim) {
  int64_t* sliceSize = ea_memdup(dimSizes, dims*sizeof(*dimSizes));
  sliceSize[sliceDim] = 1;
  int64_t* sliceOffset = calloc(dims, sizeof(*sliceOffset));

  for(sliceOffset[sliceDim] = 0; sliceOffset[sliceDim] < dimSizes[sliceDim]; sliceOffset[sliceDim]++) {
    writeFragment(dataset, dataspace, dims, sliceOffset, sliceSize, data);
  }

  free(sliceSize);
  free(sliceOffset);
}

int main(int argc, char const *argv[]) {
  // prepare data
  int64_t dimSizes[k_dims];
  for(int64_t i = 0; i < k_dims; i++) dimSizes[i] = k_edgeLength;
  uint64_t* data;
  fakeData(k_dims, dimSizes, &data);

  // Interaction with ESDM
  esdm_status ret;

  ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  // define dataspace
  esdm_dataspace_t *dataspace;
  ret = esdm_dataspace_create(k_dims, dimSizes, SMD_DTYPE_UINT64, &dataspace);
  eassert(ret == ESDM_SUCCESS);

  esdm_container_t *container;
  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);

  esdm_dataset_t *dataset;
  ret = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  //Write data three times, each time sliced in a different direction.
  //This gives us 300 slices, of which exactly 100 slices are needed to reconstruct the entire data.
  //Each data point is stored in three different fragments, and no two data points are stored in the same three fragments.
  //To successfully reconstruct the data, all 100 slices in a single direction must be selected, and no other fragments need to be read.
  //The test is, whether the algorithm actually finds one of these three solutions.
  esdm_statistics_t before = esdm_write_stats();
  writeSliced(dataset, dataspace, k_dims, dimSizes, data, 0);
  writeSliced(dataset, dataspace, k_dims, dimSizes, data, 1);
  writeSliced(dataset, dataspace, k_dims, dimSizes, data, 2);
  esdm_statistics_t after = esdm_write_stats();

  printf("bytes requested to write = %"PRId64" (expected %"PRId64")\n", after.bytesUser - before.bytesUser, 3*totalSize(k_dims, dimSizes));
  eassert(after.bytesUser - before.bytesUser == 3*totalSize(k_dims, dimSizes));
  printf("bytes written to disk = %"PRId64" (expected %"PRId64")\n", after.bytesIo - before.bytesIo, 3*totalSize(k_dims, dimSizes));
  eassert(after.bytesIo - before.bytesIo == 3*totalSize(k_dims, dimSizes));
  printf("write requests = %"PRId64" (expected %"PRId64")\n", after.requests - before.requests, 3*k_edgeLength);
  eassert(after.requests - before.requests == 3*k_edgeLength);
  printf("fragments written = %"PRId64" (expected %"PRId64")\n", after.fragments - before.fragments, 3*k_edgeLength);
  eassert(after.fragments - before.fragments == 3*k_edgeLength && "only required to have complete knowledge of the fragments on disk");

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  // Read the data from the dataset
  clearData(k_dims, dimSizes, data);
  before = esdm_read_stats();
  ret = esdm_read(dataset, data, dataspace);
  eassert(ret == ESDM_SUCCESS);
  after = esdm_read_stats();
  eassert(dataIsCorrect(k_dims, dimSizes, data));

  printf("bytes requested to read = %"PRId64" (expected %"PRId64")\n", after.bytesUser - before.bytesUser, totalSize(k_dims, dimSizes));
  eassert(after.bytesUser - before.bytesUser == totalSize(k_dims, dimSizes));
  printf("bytes read from disk = %"PRId64" (expected %"PRId64")\n", after.bytesIo - before.bytesIo, totalSize(k_dims, dimSizes));
  eassert(after.bytesIo - before.bytesIo == totalSize(k_dims, dimSizes));
  printf("read requests = %"PRId64" (expected %"PRId64")\n", after.requests - before.requests, (int64_t)1);
  eassert(after.requests - before.requests == 1);
  printf("fragments read = %"PRId64" (expected %"PRId64")\n", after.fragments - before.fragments, k_edgeLength);
  eassert(after.fragments - before.fragments == k_edgeLength);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");
}
