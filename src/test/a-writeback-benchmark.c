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

void initData(int height, int width, uint64_t (*data)[width]) {
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      data[y][x] = y*width + x;
    }
  }
}

bool dataIsCorrect(int height, int width, uint64_t (*data)[width]) {
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      if(data[y][x] != y*width + x) return false;
    }
  }
  return true;
}

//writes the data in the form of 1D slices of size [1][width]
void writeData(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int height, int width, uint64_t (*data)[width]) {
  esdm_statistics_t beforeStats = esdm_write_stats();

  int ret = ESDM_SUCCESS;
  int64_t size[2] = {1, width};
  for(int i = 0; i < height; i++) {
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
  eassert(afterStats.bytesUser - beforeStats.bytesUser == height*sizeof(*data));
//  eassert(afterStats.bytesIo - beforeStats.bytesIo == height*sizeof(*data));
  eassert(afterStats.requests - beforeStats.requests == height);
//  eassert(afterStats.fragments - beforeStats.fragments == height);
}

//fragmentSize is an array of two elements that gives the shape of the fragments to read
//expectedReadFactor gives the expected factor by which the amount of data that's read from disk is larger than the amount of data requested
void readData(esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int height, int width, uint64_t (*data)[width], int64_t* fragmentSize, int64_t expectedReadFactor, int64_t expectedRequestFactor, bool expectWriteBack) {
  eassert(height%fragmentSize[0] == 0 && "if this fails, it's a bug in the test parameterization, not in ESDM itself");
  eassert(width%fragmentSize[1] == 0 && "if this fails, it's a bug in the test parameterization, not in ESDM itself");

  esdm_statistics_t beforeStatsRead = esdm_read_stats();
  esdm_statistics_t beforeStatsWrite = esdm_write_stats();

  //perform the read
  for(int y = 0; y < height; y += fragmentSize[0]) {
    for(int x = 0; x < width; x += fragmentSize[1]) {
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

  int64_t requestCount = height/fragmentSize[0] * width/fragmentSize[1];
  int64_t fragmentCount = requestCount*expectedRequestFactor;
  eassert(afterStatsRead.bytesUser - beforeStatsRead.bytesUser == height*sizeof(*data));
//  eassert(afterStatsRead.bytesInternal - beforeStatsRead.bytesInternal == 0);
//  eassert(afterStatsRead.bytesIo - beforeStatsRead.bytesIo == expectedReadFactor*height*sizeof(*data));
  eassert(afterStatsRead.requests - beforeStatsRead.requests == requestCount);
  eassert(afterStatsRead.internalRequests - beforeStatsRead.internalRequests == 0);
//  eassert(afterStatsRead.fragments - beforeStatsRead.fragments == fragmentCount);

//  eassert(afterStatsWrite.bytesInternal - beforeStatsWrite.bytesInternal == (expectWriteBack ? height*sizeof(*data) : 0));
//  eassert(afterStatsWrite.internalRequests - beforeStatsWrite.internalRequests == (expectWriteBack ? requestCount : 0));
}

void printUsage(const char* programPath) {
  printf("Usage:\n");
  printf("%s [-?|--help] [(-s|--size-exponent) S] [(-w|--width-exponent) W] [(-h|--height-exponent) H]\n", programPath);
  printf("\n");
  printf("\t-s S, --width-exponent S\n");
  printf("\t\tset the size of the data matrix to 2^Sx2^S\n");
  printf("\n");
  printf("\t-w W, --width-exponent W\n");
  printf("\t\tset the width of the data matrix to 2^W\n");
  printf("\n");
  printf("\t-h H, --height-exponent H\n");
  printf("\t\tset the height of the data matrix to 2^H\n");
}

//argv[0] is expected to be the option name, argv[1] the integer that we need to parse
long readIntArg(int argc, char const **argv) {
  if(argc < 2) {
    fprintf(stderr, "error: %s option needs an integer argument\n", *argv);
    exit(1);
  }
  char* endPtr;
  long result = strtol(argv[1], &endPtr, 0);
  if(!*argv[1] || *endPtr) {
    fprintf(stderr, "error: the argument \"%s\" to the %s option is not an integer\n", argv[1], argv[0]);
    exit(1);
  }
  return result;
}

void readArgs(int argc, char const **argv, int* out_height, int* out_width) {
  //save the program name
  eassert(argc > 0);
  char const* programPath = *argv++;
  argc--;

  //defaults
  long heightExponent = 8, widthExponent = 8;

  for(; argc > 0; argc--, argv++) {
    if(!strcmp(*argv, "-w") || !strcmp(*argv, "--width-exponent")) {
      widthExponent = readIntArg(argc--, argv++); //gobble up an additional argument;
    } else if(!strcmp(*argv, "-h") || !strcmp(*argv, "--height-exponent")) {
      heightExponent = readIntArg(argc--, argv++);  //gobble up an additional argument;
    } else if(!strcmp(*argv, "-s") || !strcmp(*argv, "--size-exponent")) {
      widthExponent = heightExponent = readIntArg(argc--, argv++);  //gobble up an additional argument;
    } else if(!strcmp(*argv, "-?") || !strcmp(*argv, "--help")) {
      printUsage(programPath);
      exit(0);
    } else {
      fprintf(stderr, "error: unrecognized option \"%s\"\n\n", *argv);
      printUsage(programPath);
      exit(1);
    }
  }

  *out_height = 1 << heightExponent;
  *out_width = 1 << widthExponent;
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

void runTestWithConfig(int height, int width, const char* configString) {
  esdm_status ret = esdm_load_config_str(configString);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  esdm_loglevel(ESDM_LOGLEVEL_WARNING); //stop the esdm_mkfs() call from spamming us with infos about deleted objects
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);
  esdm_loglevel(ESDM_LOGLEVEL_INFO);

  printf("\nTest 1: Measure worst case reading and the effect of writeback in this case\n");

  // define dataspace
  int64_t bounds[2] = {height, width};
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

  // perform the transposition test
  uint64_t (*data)[width] = malloc(height*sizeof(*data));
  initData(height, width, data);

  timer myTimer;
  start_timer(&myTimer);
  writeData(dataset1, dataspace, height, width, data);
  printf("write data (%dx%d) as 1x%d fragments: %.3fms\n", height, width, width, 1000*stop_timer(myTimer));

  memset(data, 0, height*sizeof(*data));
  start_timer(&myTimer);
  readData(dataset1, dataspace, height, width, data, (int64_t[2]){ 1, width}, 1, 1, false);
  printf("read data as written: %.3fms\n", 1000*stop_timer(myTimer));
  eassert(dataIsCorrect(height, width, data));

  memset(data, 0, height*sizeof(*data));
  start_timer(&myTimer);
  readData(dataset1, dataspace, height, width, data, (int64_t[2]){ height, 1}, width, height, true);
  printf("read data as %dx1 fragments: %.3fms\n", height, 1000*stop_timer(myTimer));
  eassert(dataIsCorrect(height, width, data));

  memset(data, 0, height*sizeof(*data));
  start_timer(&myTimer);
  readData(dataset1, dataspace, height, width, data, (int64_t[2]){ height, 1}, 1, 1, false);
  printf("read data %dx1 fragments repeat: %.3fms\n", height, 1000*stop_timer(myTimer));
  eassert(dataIsCorrect(height, width, data));

  esdm_dataset_close(dataset1);


  printf("\nTest 2: Profile successive change of fragment shape\n");

  // Perform gradual transposition test
  esdm_dataset_t* dataset2;
  ret = esdm_dataset_create(container, "dataset2", dataspace, &dataset2);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_commit(dataset2);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  start_timer(&myTimer);
  writeData(dataset2, dataspace, height, width, data);
  printf("write data (%dx%d) as 1x%d fragments: %.3fms\n", height, width, width, 1000*stop_timer(myTimer));

  timer outerTimer;
  start_timer(&outerTimer);
  for(int64_t fragmentSize[2] = {1, width}, readFactor = 1; fragmentSize[1] && fragmentSize[0] <= height; fragmentSize[1] /= 2, fragmentSize[0] *=2, readFactor *= 2) {
    memset(data, 0, height*sizeof(*data));
    bool expectWriteback = readFactor >= 8;
    start_timer(&myTimer);
    readData(dataset2, dataspace, height, width, data, fragmentSize, readFactor, readFactor, expectWriteback);
    printf("read data as %"PRId64"x%"PRId64" fragments: %.3fms%s\n", fragmentSize[0], fragmentSize[1], 1000*stop_timer(myTimer), expectWriteback ? " (writeback)" : "");
    if(expectWriteback) readFactor = 1;
    eassert(dataIsCorrect(height, width, data));
  }
  free(data);
  printf("total: %.3fms\n", 1000*stop_timer(outerTimer));

  esdm_dataspace_destroy(dataspace);
  esdm_dataset_close(dataset2);
  esdm_container_close(container);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);
}

int main(int argc, char const *argv[]) {
  int height, width;
  readArgs(argc, argv, &height, &width);

  printf("=== array based bound list ===\n\n");
  runTestWithConfig(height, width, "{ \"esdm\": { \"bound list implementation\": \"array\", \"backends\": [ { \"type\": \"POSIX\", \"id\": \"p1\", \"accessibility\": \"global\", \"target\": \"./_posix1\" } ], \"metadata\": { \"type\": \"metadummy\", \"id\": \"md\", \"target\": \"./_metadummy\" } } }");

  printf("\n\n=== B-tree based bound list ===\n\n");
  runTestWithConfig(height, width, "{ \"esdm\": { \"bound list implementation\": \"btree\", \"backends\": [ { \"type\": \"POSIX\", \"id\": \"p1\", \"accessibility\": \"global\", \"target\": \"./_posix1\" } ], \"metadata\": { \"type\": \"metadummy\", \"id\": \"md\", \"target\": \"./_metadummy\" } } }");

  printf("\n\n=== array based bound list (repeat) ===\n\n");
  runTestWithConfig(height, width, "{ \"esdm\": { \"bound list implementation\": \"array\", \"backends\": [ { \"type\": \"POSIX\", \"id\": \"p1\", \"accessibility\": \"global\", \"target\": \"./_posix1\" } ], \"metadata\": { \"type\": \"metadummy\", \"id\": \"md\", \"target\": \"./_metadummy\" } } }");

  printf("\nOK\n");

  return 0;
}
