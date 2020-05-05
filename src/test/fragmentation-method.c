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
#include <stdbool.h>
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

bool dataIsCorrect(int64_t dims, int64_t* dimSizes, uint64_t* data) {
  uint64_t* referenceData;
  fakeData(dims, dimSizes, &referenceData);
  bool result = !memcmp(data, referenceData, totalSize(dims, dimSizes));
  free(referenceData);
  return result;
}

void writeData(int64_t dims, int64_t* dimSizes, esdm_dataset_t* dataset, esdm_dataspace_t* dataspace) {
  uint64_t* data;
  fakeData(dims, dimSizes, &data);
  esdm_status ret = esdm_write(dataset, data, dataspace);
  eassert(ret == ESDM_SUCCESS);
  free(data);
}

void readAndCheckData(int64_t dims, int64_t* dimSizes, esdm_dataset_t* dataset, esdm_dataspace_t* dataspace) {
  uint64_t* data = malloc(totalSize(dims, dimSizes));
  esdm_status ret = esdm_read(dataset, data, dataspace);
  eassert(ret == ESDM_SUCCESS);
  eassert(dataIsCorrect(dims, dimSizes, data));
  free(data);
}

void testMain(int64_t dims, int64_t* dimSizes, int64_t threads, int64_t nodeThreads, int64_t maxFragmentSize, bool contiguousFragmentation, int64_t expectedFragmentCount) {
  char* config = NULL;
  size_t configSize;
  FILE* stream = open_memstream(&config, &configSize);
  fprintf(stream, "{ \"esdm\": { \"backends\": [ { \"type\": \"POSIX\", \"id\": \"p1\", \"max-threads-per-node\": %"PRId64", \"max-fragment-size\": %"PRId64", \"fragmentation-method\": \"%s\", \"max-global-threads\": %"PRId64", \"accessibility\": \"global\", \"target\": \"./_posix1\" } ], \"metadata\": { \"type\": \"metadummy\", \"id\": \"md\", \"target\": \"./_metadummy\" } } }", nodeThreads, maxFragmentSize, (contiguousFragmentation ? "contiguous" : "equalized"), threads);
  fclose(stream);

  esdm_status ret = esdm_load_config_str(config);
  eassert(ret == ESDM_SUCCESS);
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

  esdm_statistics_t before = esdm_write_stats();
  writeData(dims, dimSizes, dataset, dataspace);
  esdm_statistics_t after = esdm_write_stats();

  printf("fragments written = %"PRId64" (expected %"PRId64")\n", after.fragments - before.fragments, expectedFragmentCount);
  eassert(after.fragments - before.fragments == expectedFragmentCount);

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  // Read the data from the dataset
  before = esdm_read_stats();
  readAndCheckData(dims, dimSizes, dataset, dataspace);
  after = esdm_read_stats();

  printf("fragments read = %"PRId64" (expected %"PRId64")\n", after.fragments - before.fragments, expectedFragmentCount);
  eassert(after.fragments - before.fragments == expectedFragmentCount);

  esdm_dataspace_destroy(dataspace);
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  free(config);
}

int main() {
  testMain(2, (int64_t[]){46, 466}, 0, 400, 2000, false, 4*32);
  testMain(2, (int64_t[]){45, 465}, 0, 400, 2000, false, 3*31);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 50, true, 10*10*2);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 100, true, 10*10);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 200, true, 10*5);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 400, true, 10*2);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 800, true, 10);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 1000, true, 10);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 2000, true, 5);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 3000, true, 4);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 4000, true, 2);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 7999, true, 2);
  testMain(3, (int64_t[]){10, 10, 10}, 0, 400, 8000, true, 1);
}
