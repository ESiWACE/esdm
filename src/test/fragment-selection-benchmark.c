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
#define k_edgeLength 100
int64_t k_dataSize[] = {k_edgeLength, k_edgeLength, k_edgeLength};

void fakeData(uint64_t (**out_data)[k_edgeLength][k_edgeLength]) {
  *out_data = malloc(k_edgeLength*sizeof(**out_data));
  for(int z = 0; z < k_edgeLength; z++) {
    for(int y = 0; y < k_edgeLength; y++) {
      for(int x = 0; x < k_edgeLength; x++) {
        (*out_data)[z][y][x] = (z*k_edgeLength + y)*k_edgeLength + x;;
      }
    }
  }
}

void clearData(uint64_t (*data)[k_edgeLength][k_edgeLength]) {
  memset(data, 0, k_edgeLength*sizeof(*data));
}

bool dataIsCorrect(uint64_t (*data)[k_edgeLength][k_edgeLength]) {
  static uint64_t (*referenceData)[k_edgeLength][k_edgeLength] = NULL;
  if(!referenceData) fakeData(&referenceData);
  return !memcmp(data, referenceData, k_edgeLength*sizeof(*referenceData));
}

void writeFragment(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t offset[k_dims], int64_t size[k_dims], uint64_t (*data)[k_edgeLength][k_edgeLength]) {
  esdm_dataspace_t* subspace;
  int ret = esdm_dataspace_subspace(dataspace, k_dims, size, offset, &subspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataspace_copyDatalayout(subspace, dataspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_write(dataset, (char*)data + esdm_dataspace_elementOffset(dataspace, offset), subspace);
  eassert(ret == ESDM_SUCCESS);
}

void writeSliced(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t sliceDim, uint64_t (*data)[k_edgeLength][k_edgeLength]) {
  int64_t* sliceSize = ea_memdup(k_dataSize, sizeof(k_dataSize));
  sliceSize[sliceDim] = 1;
  int64_t* sliceOffset = calloc(k_dims, sizeof(*sliceOffset));;

  for(sliceOffset[sliceDim] = 0; sliceOffset[sliceDim] < k_edgeLength; sliceOffset[sliceDim]++) {
    writeFragment(dataset, dataspace, sliceOffset, sliceSize, data);
  }

  free(sliceSize);
  free(sliceOffset);
}

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t (*data)[k_edgeLength][k_edgeLength];
  fakeData(&data);

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
  ret = esdm_dataspace_create(k_dims, k_dataSize, SMD_DTYPE_UINT64, &dataspace);
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
  writeSliced(dataset, dataspace, 0, data);
  writeSliced(dataset, dataspace, 1, data);
  writeSliced(dataset, dataspace, 2, data);

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  // Read the data from the dataset
  clearData(data);
  ret = esdm_read(dataset, data, dataspace);
  eassert(ret == ESDM_SUCCESS);
  eassert(dataIsCorrect(data));

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");
}
