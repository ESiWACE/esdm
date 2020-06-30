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
 * This is a stress test for the handling of many short dimensions.
 */


#include <esdm.h>
#include <esdm-internal.h>
#include <test/util/test_util.h>

#include <stdio.h>
#include <stdlib.h>

void printUsage(const char* programPath) {
  printf("Usage:\n");
  printf("%s [(-c|--count) C] [-?|-h|--help]\n", programPath);
  printf("\n");
  printf("\t-c C, --count C\n");
  printf("\t\tSet the count of dimensions to use.\n");
  printf("\t\tTotal data size is 2^C elements of eight bytes.\n");
  printf("\n");
  printf("\t-?, -h, --help\n");
  printf("\t\tPrint this usage information and exit.\n");
}

//argv[0] is expected to be the option name, argv[1] the integer that we need to parse
int64_t readIntArg(int argc, char const **argv) {
  if(argc < 2) {
    fprintf(stderr, "error: %s option needs an integer argument\n", *argv);
    exit(1);
  }
  char* endPtr;
  int64_t result = strtol(argv[1], &endPtr, 0);
  if(!*argv[1] || *endPtr) {
    fprintf(stderr, "error: the argument \"%s\" to the %s option is not an integer\n", argv[1], argv[0]);
    exit(1);
  }
  return result;
}

void readArgs(int argc, char const **argv, int64_t* out_dimCount) {
  //save the program name
  eassert(argc > 0);
  char const* programPath = *argv++;
  argc--;

  //defaults
  *out_dimCount = 11;  //2048 fragments

  for(; argc > 0; argc--, argv++) {
    if(!strcmp(*argv, "-c") || !strcmp(*argv, "--count")) {
      *out_dimCount = readIntArg(argc--, argv++);  //gobble up an additional argument;
    } else if(!strcmp(*argv, "-?") || !strcmp(*argv, "-h") || !strcmp(*argv, "--help")) {
      printUsage(programPath);
      exit(0);
    } else {
      fprintf(stderr, "error: unrecognized option \"%s\"\n\n", *argv);
      printUsage(programPath);
      exit(1);
    }
  }
}

int main(int argc, char const **argv) {
  int64_t dimCount;
  readArgs(argc, argv, &dimCount);

  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  esdm_loglevel(ESDM_LOGLEVEL_WARNING); //stop the esdm_mkfs() call from spamming us with infos about deleted objects
  timer myTimer;
  ea_start_timer(&myTimer);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  printf("esdm_mkfs(): %.3fms\n", 1000*ea_stop_timer(myTimer));
  ea_start_timer(&myTimer);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);
  printf("esdm_mkfs(): %.3fms\n", 1000*ea_stop_timer(myTimer));
  esdm_loglevel(ESDM_LOGLEVEL_INFO);

  //define dataset
  int64_t size[dimCount];
  for(int64_t i = dimCount; i--; ) size[i] = 2;
  esdm_dataspace_t* dataspace;
  ret = esdm_dataspace_create(dimCount, size, SMD_DTYPE_UINT64, &dataspace);
  eassert(ret == ESDM_SUCCESS);
  esdm_container_t* container;
  ret = esdm_container_create("many-dims-stress-test", true, &container);
  eassert(ret == ESDM_SUCCESS);
  esdm_dataset_t* dataset;
  ret = esdm_dataset_create(container, "dataset", dataspace, &dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  //write data
  int64_t fragmentSize[dimCount];
  for(int64_t i = dimCount; i--; ) fragmentSize[i] = 1;
  for(int64_t i = 1 << dimCount; i--; ) {
    int64_t offset[dimCount];
    for(int64_t dim = dimCount; dim--; ) offset[dim] = (i >> (dimCount - dim - 1))&1;
    esdm_dataspace_t* subspace;
    ret = esdm_dataspace_subspace(dataspace, dimCount, fragmentSize, offset, &subspace);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_write(dataset, &i, subspace);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_destroy(subspace);
    eassert(ret == ESDM_SUCCESS);
  }

  //read data
  int64_t* data = malloc((1 << dimCount)*sizeof*data);
  ret = esdm_read(dataset, data, dataspace);
  eassert(ret == ESDM_SUCCESS);

  //check data
  for(int64_t i = 1 << dimCount; i--; ) {
    if(data[i] != i) {
      fprintf(stderr, "error in data detected: 0x%016"PRIx64" expected, found 0x%016"PRIx64"\n", i, data[i]);
      abort();
    }
  }

  //cleanup
  free(data);
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);
}
