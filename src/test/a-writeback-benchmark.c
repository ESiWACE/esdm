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
#include <esdm-internal.h>
#include <test/util/test_util.h>

#include <stdio.h>
#include <stdlib.h>

#define N (1 << 8)

void initData(uint64_t (*data)[N]) {
  for(int y = 0; y < N; y++) {
    for(int x = 0; x < N; x++) {
      data[y][x] = y*N + x;
    }
  }
}

bool dataIsCorrect(uint64_t (*data)[N]) {
  for(int y = 0; y < N; y++) {
    for(int x = 0; x < N; x++) {
      if(data[y][x] != y*N + x) return false;
    }
  }
  return true;
}

//writes the data in the form of 1D slices of size [1][N]
void writeData(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, uint64_t (*data)[N]) {
  esdm_statistics_t beforeStats = esdm_write_stats();

  int ret = ESDM_SUCCESS;
  int64_t size[2] = {1, N};
  for(int i = 0; i < N; i++) {
    int64_t offset[2] = {i, 0};
    esdm_dataspace_t *subspace;
    ret = esdm_dataspace_subspace(dataspace, 2, size, offset, &subspace);
    eassert(ret == ESDM_SUCCESS);

    ret = esdm_write(dataset, data[i], subspace);
    eassert(ret == ESDM_SUCCESS);

    ret = esdm_dataspace_destroy(subspace);
    eassert(ret == ESDM_SUCCESS);
  }

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  //check statistics
  esdm_statistics_t afterStats = esdm_write_stats();
  eassert(afterStats.bytesUser - beforeStats.bytesUser == N*sizeof(*data));
  eassert(afterStats.bytesIo - beforeStats.bytesIo == N*sizeof(*data));
  eassert(afterStats.requests - beforeStats.requests == N);
  eassert(afterStats.fragments - beforeStats.fragments == N);
}

//fragmentSize is an array of two elements that gives the shape of the fragments to read
//expectedReadFactor gives the expected factor by which the amount of data that's read from disk is larger than the amount of data requested
void readData(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, uint64_t (*data)[N], int64_t* fragmentSize, int64_t expectedReadFactor, bool expectWriteBack) {
  eassert(N%fragmentSize[0] == 0 && "if this fails, it's a bug in the test parameterization, not in ESDM itself");
  eassert(N%fragmentSize[1] == 0 && "if this fails, it's a bug in the test parameterization, not in ESDM itself");

  esdm_statistics_t beforeStatsRead = esdm_read_stats();
  esdm_statistics_t beforeStatsWrite = esdm_write_stats();

  //perform the read
  for(int y = 0; y < N; y += fragmentSize[0]) {
    for(int x = 0; x < N; x += fragmentSize[1]) {
      esdm_dataspace_t* subspace;
      int64_t offset[2] = {y, x};
      int ret = esdm_dataspace_subspace(dataspace, 2, fragmentSize, offset, &subspace);
      eassert(ret == ESDM_SUCCESS);
      ret = esdm_dataspace_copyDatalayout(subspace, dataspace);
      eassert(ret == ESDM_SUCCESS);

      ret = esdm_read(dataset, &data[y][x], subspace);
      eassert(ret == ESDM_SUCCESS);

      ret = esdm_dataspace_destroy(subspace);
      eassert(ret == ESDM_SUCCESS);
    }
  }

  //check statistics
  esdm_statistics_t afterStatsRead = esdm_read_stats();
  esdm_statistics_t afterStatsWrite = esdm_write_stats();

  eassert(afterStatsRead.bytesUser - beforeStatsRead.bytesUser == N*sizeof(*data));
  eassert(afterStatsRead.bytesInternal - beforeStatsRead.bytesInternal == 0);
  eassert(afterStatsRead.bytesIo - beforeStatsRead.bytesIo == expectedReadFactor*N*sizeof(*data));
  eassert(afterStatsRead.requests - beforeStatsRead.requests == N);
  eassert(afterStatsRead.internalRequests - beforeStatsRead.internalRequests == 0);
  eassert(afterStatsRead.fragments - beforeStatsRead.fragments == expectedReadFactor*N);

  eassert(afterStatsWrite.bytesInternal - beforeStatsWrite.bytesInternal == (expectWriteBack ? N*sizeof(*data) : 0));
  eassert(afterStatsWrite.internalRequests - beforeStatsWrite.internalRequests == (expectWriteBack ? N : 0));
}

//TODO: Benchmark idea:
//      Write 2D dataset (NxN) as 1D slices.
//      Read as transposed 1D slices. Measure time and check statistics, dataset is expected to be read N times.
//      Read again in same way. Measure time and check that only the write-back fragments have been used, that the dataset is only read once.
//
//      Write another 2D NxN dataset as 1xN slices.
//      Read as 2xN/2 slices, check statistics (2x read).
//      Read as 4xN/4 slices, check statistics (4x read).
//      Read as 8xN/8 slices, check that write-back happens (8x read, 1x write).
//      Read as 16xN/16 slices, check statistics (2x read).
//      ...

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t (*data)[N] = malloc(N*sizeof(*data));
  initData(data);

  // Interaction with ESDM
  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);



  printf("\nTest 1: Measure worst case reading and the effect of writeback in this case\n");

  // define dataspace
  int64_t bounds[2] = {N, N};
  esdm_dataspace_t *dataspace;

  ret = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);
  eassert(ret == ESDM_SUCCESS);
  esdm_container_t *container;
  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);

  esdm_dataset_t *dataset1;
  ret = esdm_dataset_create(container, "dataset1", dataspace, &dataset1);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_commit(dataset1);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  timer myTimer;
  start_timer(&myTimer);
  writeData(dataset1, dataspace, data);
  printf("write data: %.3fms\n", 1000*stop_timer(myTimer));

  memset(data, 0, N*sizeof(*data));
  start_timer(&myTimer);
  readData(dataset1, dataspace, data, (int64_t[2]){ 1, N}, 1, false);
  printf("read data as written: %.3fms\n", 1000*stop_timer(myTimer));
  eassert(dataIsCorrect(data));

  memset(data, 0, N*sizeof(*data));
  start_timer(&myTimer);
  readData(dataset1, dataspace, data, (int64_t[2]){ N, 1}, N, true);
  printf("read data worst case: %.3fms\n", 1000*stop_timer(myTimer));
  eassert(dataIsCorrect(data));

  memset(data, 0, N*sizeof(*data));
  start_timer(&myTimer);
  readData(dataset1, dataspace, data, (int64_t[2]){ N, 1}, 1, false);
  printf("read data worst case repeat: %.3fms\n", 1000*stop_timer(myTimer));
  eassert(dataIsCorrect(data));



  printf("\nTest 2: Profile successive change of fragment shape\n");

  //TODO Perform gradual transposition test
  esdm_dataset_t* dataset2;
  ret = esdm_dataset_create(container, "dataset2", dataspace, &dataset2);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_commit(dataset2);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  start_timer(&myTimer);
  writeData(dataset2, dataspace, data);
  printf("write data: %.3fms\n", 1000*stop_timer(myTimer));

  timer outerTimer;
  start_timer(&outerTimer);
  for(int64_t width = N, hight = 1, readFactor = 1; width; width /= 2, hight *=2, readFactor *= 2) {
    memset(data, 0, N*sizeof(*data));
    bool expectWriteback = readFactor >= 8;
    start_timer(&myTimer);
    readData(dataset2, dataspace, data, (int64_t[2]){ hight, width}, readFactor, expectWriteback);
    printf("read data as %"PRId64"x%"PRId64" fragments: %.3fms%s\n", hight, width, 1000*stop_timer(myTimer), expectWriteback ? " (writeback)" : "");
    if(expectWriteback) readFactor = 1;
    eassert(dataIsCorrect(data));
  }
  printf("total: %.3fms\n", 1000*stop_timer(outerTimer));

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");

  return 0;
}
