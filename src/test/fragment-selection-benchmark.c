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
  ret = esdm_write(dataset, (char*)data + esdm_dataspace_elementOffset(dataspace, offset), subspace, NULL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataspace_destroy(subspace);
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

void exitWithUsage(const char* executableName, char* errorMessage) {
  if(errorMessage) fprintf(stderr, "%s\n", errorMessage);
  printf("usage: %s [(-d | --dims) dims] [(-e | --edge-size) edgeSize]\n", executableName);
  printf("\n");
  printf("\t(-d | --dims) dims\n");
  printf("\t\tUse `dims` dimensions for the data hypercube.\n");
  printf("\n");
  printf("\t(-e | --edge-size) edgeSize\n");
  printf("\t\tSet the length of all the sides of the data hypercube to `edgeSize`.\n");
  exit(-!!errorMessage);
}

void parseArgs(int argc, char const** argv, int64_t* out_dims, int64_t* out_edgeLength) {
  assert(argc > 0);
  const char* executableName = argv[0];

  //set defaults
  *out_dims = 3;
  *out_edgeLength = 100;

  //parse the args
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dims")) {
      if(++i >= argc) exitWithUsage(executableName, "error: -d | --dims option needs an argument\n");
      char* endPtr;
      *out_dims = strtoll(argv[i], &endPtr, 0);
      if(*endPtr) exitWithUsage(executableName, "error: argument to -d | --dims option needs to be an integer\n");
    } else if(!strcmp(argv[i], "-e") || !strcmp(argv[i], "--edge-size")) {
      if(++i >= argc) exitWithUsage(executableName, "error: -e | --edge-size option needs an argument\n");
      char* endPtr;
      *out_edgeLength = strtoll(argv[i], &endPtr, 0);
      if(*endPtr) exitWithUsage(executableName, "error: argument to -e | --edge-size option needs to be an integer\n");
    } else {
      exitWithUsage(executableName, "error: unrecognized option\n");
    }
  }
}

int main(int argc, char const *argv[]) {
  // parse the command line args
  int64_t dims, edgeLength;
  parseArgs(argc, argv, &dims, &edgeLength);

  // prepare data
  int64_t dimSizes[dims];
  for(int64_t i = 0; i < dims; i++) dimSizes[i] = edgeLength;
  uint64_t* data;
  fakeData(dims, dimSizes, &data);

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
  ret = esdm_dataspace_create(dims, dimSizes, SMD_DTYPE_UINT64, &dataspace);
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
  for(int64_t sliceDim = 0; sliceDim < dims; sliceDim++) writeSliced(dataset, dataspace, dims, dimSizes, data, sliceDim);
  esdm_statistics_t after = esdm_write_stats();

  printf("bytes requested to write = %"PRId64" (expected %"PRId64")\n", after.bytesUser - before.bytesUser, dims*totalSize(dims, dimSizes));
  eassert(after.bytesUser - before.bytesUser == dims*totalSize(dims, dimSizes));
  printf("bytes written to disk = %"PRId64" (expected %"PRId64")\n", after.bytesIo - before.bytesIo, dims*totalSize(dims, dimSizes));
  eassert(after.bytesIo - before.bytesIo == dims*totalSize(dims, dimSizes));
  printf("write requests = %"PRId64" (expected %"PRId64")\n", after.requests - before.requests, dims*edgeLength);
  eassert(after.requests - before.requests == dims*edgeLength);
  printf("fragments written = %"PRId64" (expected %"PRId64")\n", after.fragments - before.fragments, dims*edgeLength);
  eassert(after.fragments - before.fragments == dims*edgeLength && "only required to have complete knowledge of the fragments on disk");

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  // Read the data from the dataset
  clearData(dims, dimSizes, data);
  before = esdm_read_stats();
  ret = esdm_read(dataset, data, dataspace, NULL);
  eassert(ret == ESDM_SUCCESS);
  after = esdm_read_stats();
  eassert(dataIsCorrect(dims, dimSizes, data));
  free(data);

  printf("bytes requested to read = %"PRId64" (expected %"PRId64")\n", after.bytesUser - before.bytesUser, totalSize(dims, dimSizes));
  eassert(after.bytesUser - before.bytesUser == totalSize(dims, dimSizes));
  printf("bytes read from disk = %"PRId64" (expected %"PRId64")\n", after.bytesIo - before.bytesIo, totalSize(dims, dimSizes));
  eassert(after.bytesIo - before.bytesIo == totalSize(dims, dimSizes));
  printf("read requests = %"PRId64" (expected %"PRId64")\n", after.requests - before.requests, (int64_t)1);
  eassert(after.requests - before.requests == 1);
  printf("fragments read = %"PRId64" (expected %"PRId64")\n", after.fragments - before.fragments, edgeLength);
  eassert(after.fragments - before.fragments == edgeLength);

  esdm_dataspace_destroy(dataspace);
  esdm_dataset_close(dataset);
  esdm_container_close(container);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");
}
