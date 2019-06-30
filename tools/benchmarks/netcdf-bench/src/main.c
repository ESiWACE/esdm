/*
 * =====================================================================================
 *
 *       Filename:  netcdf-bench.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/17/2016 11:41:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */

#include <assert.h>
#include <libgen.h>
#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "benchmark.h"
#include "constants.h"
#include "debug.h"
#include "options.h"
#include "report.h"
#include "types.h"


unsigned int to_char(const long long value, const size_t pos) {
  const long long r = value / ((long long)pow(10, pos)); // remove the last positions, e.g. pos=2 and value=2136 -> r=21
  const long long x = r / 10 * 10;                       // set last position to zero, e.g. r=21 -> x=20
  const long long res = r - x;                           // e.g. r-x = 21-20 = 1
  return res;
}

char *create_pretty_number_ll(const double value) {
  char *names[] = {"", "k", "M", "G", "T", "P", "E", "Z", "Y"};
  char *pretty_number;
  size_t pretty_number_size;
  FILE *stream = open_memstream(&pretty_number, &pretty_number_size);

  long long int a = (long long)value;
  double b = value - a;

  int n = floor(log10(a));
  for (size_t pos = n; pos >= n / 3 * 3; --pos) {
    fprintf(stream, "%d", to_char(value, pos));
  }
  fprintf(stream, ".");
  for (size_t pos = n / 3 * 3 - 1; pos > n / 3 * 3 - 2; --pos) {
    fprintf(stream, "%d", to_char(value, pos));
  }

  fprintf(stream, " %s%s", names[n / 3], "B");
  fclose(stream);

  return pretty_number;
}

void destroy_pretty_number_ll(char *number) {
  free(number);
  number = NULL;
}


/**
 * @brief
 *
 * @param s
 * @param dims
 * @param size
 */
static void parse_dims(const char *s, size_t **dims, size_t *size) {
  if (s == NULL) {
    return;
  }
  const char *seps = ":";
  char sep = seps[0];
  size_t len = strlen(s);
  char *s_copy = malloc(sizeof(char *) * len + 1);
  strcpy(s_copy, s);

  // Count separators
  (*size) = 0;
  for (size_t i = 0; i < len; ++i) {
    if (s_copy[i] == sep) {
      ++(*size);
    }
  }
  ++(*size);
  *dims = (size_t *)malloc(*size * sizeof(*dims));

  // Get values
  char *pch = strtok(s_copy, seps);
  char *end = NULL;
  size_t i = 0;
  while (pch != NULL) {
    (*dims)[i] = (size_t)strtol(pch, &end, 10);
    pch = strtok(NULL, seps);
    ++i;
  }
  free(s_copy);

  // Sanity checks
  if (i != *size) {
    FATAL_ERR("Couldn't parse dims from %s. Correct format is t:x:y:z\n", s);
  }

  for (size_t i = 0; i < *size; ++i) {
    if ((*dims)[i] <= 0) {
      FATAL_ERR("Bad dimension specifications in %s (parsed value is <0)\n", s);
    }
  }
}


struct args {
  procs_t procs;
  size_t *dgeom;
  size_t dgeom_size;
  size_t *cgeom;
  size_t cgeom_size;
  size_t *bgeom;
  size_t bgeom_size;
  char *testfn;
  int write_test;
  int read_test;
  report_type_t report_type;
  int par_access;
  int is_unlimited;
  int verify;
  int fill_value;
};

void print_header(benchmark_t *bm) {
  char buffer[200];
  const int dist2 = 40;
  const int dist3 = 20;
  const int dist4 = 20;
  const size_t dsize = bm->dgeom[0] * bm->dgeom[1] * bm->dgeom[2] * bm->dgeom[3] * sizeof(bm->block[0]);
  const size_t bsize = bm->bgeom[0] * bm->bgeom[1] * bm->bgeom[2] * bm->bgeom[3] * sizeof(bm->block[0]);
  size_t csize = 0;
  if (NC_CHUNKED == bm->storage) {
    csize = bm->cgeom[0] * bm->cgeom[1] * bm->cgeom[2] * bm->cgeom[3] * sizeof(bm->block[0]);
  }
  snprintf(buffer, 200, "%zu:%zu:%zu:%zu x %zu", bm->dgeom[0], bm->dgeom[1], bm->dgeom[2], bm->dgeom[3], sizeof(bm->block[0]));
  printf("%-*s %*s %-*s\n", dist2, "Data geometry (t:x:y:z x sizeof(type))", dist3, buffer, dist4, "bytes");
  snprintf(buffer, 200, "%zu:%zu:%zu:%zu x %zu", bm->bgeom[0], bm->bgeom[1], bm->bgeom[2], bm->bgeom[3], sizeof(bm->block[0]));
  printf("%-*s %*s %-*s\n", dist2, "Block geometry (t:x:y:z x sizeof(type))", dist3, buffer, dist4, "bytes");
  if (NC_CHUNKED == bm->storage) {
    snprintf(buffer, 200, "%zu:%zu:%zu:%zu x %zu", bm->cgeom[0], bm->cgeom[1], bm->cgeom[2], bm->cgeom[3], sizeof(bm->block[0]));
    printf("%-*s %*s %-*s\n", dist2, "Chunk geometry (t:x:y:z x sizeof(type))", dist3, buffer, dist4, "bytes");
  }
  char *pretty_dsize = create_pretty_number_ll(dsize);
  printf("%-*s %*.zu %-*s (%s)\n", dist2, "Datasize", dist3, dsize, dist4, "bytes", pretty_dsize);
  destroy_pretty_number_ll(pretty_dsize);

  char *pretty_bsize = create_pretty_number_ll(bsize);
  printf("%-*s %*.zu %-*s (%s)\n", dist2, "Blocksize", dist3, bsize, dist4, "bytes", pretty_bsize);
  destroy_pretty_number_ll(pretty_bsize);
  if (NC_CHUNKED == bm->storage) {
    char *pretty_csize = create_pretty_number_ll(csize);
    printf("%-*s %*.zu %-*s (%s)\n", dist2, "Chunksize", dist3, csize, dist4, "bytes", pretty_csize);
    destroy_pretty_number_ll(pretty_csize);
  }

  switch (bm->par_access) {
    case NC_COLLECTIVE:
      printf("%-*s %*s\n", dist2, "I/O Access", dist3, "collective");
      break;
    case NC_INDEPENDENT:
      printf("%-*s %*s\n", dist2, "I/O Access", dist3, "independent");
      break;
  }
  switch (bm->storage) {
    case NC_CHUNKED:
      printf("%-*s %*s\n", dist2, "Storage", dist3, "chunked");
      break;
    case NC_CONTIGUOUS:
      printf("%-*s %*s\n", dist2, "Storage", dist3, "contiguous");
      break;
  }

  if (bm->is_unlimited) {
    printf("%-*s %*s\n", dist2, "File length", dist3, "unlimited");
  } else {
    printf("%-*s %*s\n", dist2, "File length", dist3, "fixed");
  }

  if (bm->use_fill_value) {
    printf("%-*s %*s\n", dist2, "Fill value", dist3, "yes");
  } else {
    printf("%-*s %*s\n", dist2, "File value", dist3, "no");
  }
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);


  // Default values
  struct args args;
  args.procs.nn = 0;
  args.procs.ppn = 0;
  args.dgeom_size = 0;
  args.dgeom = NULL;
  args.bgeom_size = 0;
  args.bgeom = NULL;
  args.cgeom_size = 0;
  args.cgeom = NULL;
  args.testfn = NULL;
  args.write_test = 0;
  args.read_test = 0;
  args.report_type = REPORT_HUMAN;
  args.par_access = NC_INDEPENDENT;
  args.is_unlimited = 0;
  args.verify = 0;
  args.fill_value = 0;

  char *dg = NULL, *bg = NULL, *cg = NULL, *iot = "ind", *xf = "human";

  option_help options[] = {
  {'n', "nn", "Number of nodes", OPTION_OPTIONAL_ARGUMENT, 'd', &args.procs.nn},
  {'p', "ppn", "Number of processes", OPTION_OPTIONAL_ARGUMENT, 'd', &args.procs.ppn},
  {'d', "data-geometry", "Data geometry (t:x:y:z)", OPTION_OPTIONAL_ARGUMENT, 's', &dg},
  {'b', "block-geometry", "Block geometry (t:x:y:z)", OPTION_OPTIONAL_ARGUMENT, 's', &bg},
  {'c', "chunk-geometry", "Chunk geometry (t:x:y:z|auto)", OPTION_OPTIONAL_ARGUMENT, 's', &cg},
  {'r', "read", "Enable read benchmark", OPTION_FLAG, 'd', &args.read_test},
  {'w', "write", "Enable write benchmark", OPTION_FLAG, 'd', &args.write_test},
  {'t', "io-type", "Independent / Collective I/O (ind|coll)", OPTION_OPTIONAL_ARGUMENT, 's', &iot},
  {'u', "unlimited", "Enable unlimited time dimension", OPTION_FLAG, 'd', &args.is_unlimited},
  {'f', "testfile", "Filename of the testfile", OPTION_OPTIONAL_ARGUMENT, 's', &args.testfn},
  {'x', "output-format", "Output-Format (parser|human)", OPTION_OPTIONAL_ARGUMENT, 's', &xf},
  {'F', "use-fill-value", "Write a fill value", OPTION_FLAG, 'd', &args.fill_value},
  {0, "verify", "Verify that the data read is correct (reads the data again)", OPTION_FLAG, 'd', &args.verify},
  LAST_OPTION};
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // check the correctness of the options only for rank 0
  if (rank == 0) {
    printf("Benchtool (type: %s) \n", xstr(DATATYPE));
    parseOptions(argc, argv, options);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank != 0) {
    parseOptions(argc, argv, options);
  }

  parse_dims(dg, &args.dgeom, &args.dgeom_size);
  parse_dims(bg, &args.bgeom, &args.bgeom_size);

  if ((0 == strcmp(iot, "c")) | (0 == strcmp(iot, "coll")) | (0 == strcmp(iot, "collective"))) {
    args.par_access = NC_COLLECTIVE;
  } else if ((0 == strcmp(iot, "i")) | (0 == strcmp(iot, "ind")) | (0 == strcmp(iot, "independent"))) {
    args.par_access = NC_INDEPENDENT;
  } else {
    FATAL_ERR("Unsupported parallel access type %s\n", xf);
  }

  if (0 == strcmp(xf, "parser")) {
    args.report_type = REPORT_PARSER;
  } else if (0 == strcmp(xf, "human")) {
    args.report_type = REPORT_HUMAN;
  } else {
    FATAL_ERR("Unsupported report type %s\n", xf);
  }

  if (0 == args.procs.nn) {
    char *end = NULL;
    const char *env = getenv("SLURM_NNODES");
    if (NULL != env) {
      args.procs.nn = strtol(env, &end, 10);
    }
    if (0 == args.procs.nn) {
      args.procs.nn = 1;
    }
  }

  if (0 == args.procs.ppn) {
    char *end = NULL;
    const char *env = getenv("SLURM_NTASKS_PER_NODE");
    if (NULL != env) {
      args.procs.ppn = strtol(env, &end, 10);
    }
    if (0 == args.procs.ppn) {
      args.procs.ppn = 1;
    }
  }

  if (NULL == args.testfn) {
    const char *testfn = "./testfn.nc";
    args.testfn = (char *)malloc(sizeof(*args.testfn) * strlen(testfn) + 1);
    strcpy(args.testfn, testfn);
  }

  if (NULL == args.dgeom) {
    args.dgeom_size = NDIMS;
    args.dgeom = (size_t *)malloc(sizeof(*args.dgeom) * args.dgeom_size);
    args.dgeom[DT] = 100;
    args.dgeom[DX] = args.procs.nn * 100;
    args.dgeom[DY] = args.procs.ppn * 100;
    args.dgeom[DZ] = 10;
  }

  if (NDIMS != args.dgeom_size) {
    FATAL_ERR("Found %zu dims (expected %d).\n", args.dgeom_size, NDIMS);
  }

  // Automatic block layout
  if (NULL == args.bgeom) {
    args.bgeom_size = args.dgeom_size;
    args.bgeom = (size_t *)malloc(sizeof(*args.bgeom) * args.bgeom_size);
    args.bgeom[DT] = 1;
    args.bgeom[DX] = args.dgeom[DX] / args.procs.nn;
    args.bgeom[DY] = args.dgeom[DY] / args.procs.ppn;
    args.bgeom[DZ] = args.dgeom[DZ];
  }

  if (cg != NULL && 0 == strcmp(cg, "auto")) {
    args.cgeom_size = args.bgeom_size;
    args.cgeom = (size_t *)malloc(sizeof(*args.cgeom) * args.cgeom_size);
    args.cgeom[DT] = 1;
    args.cgeom[DX] = args.bgeom[DX];
    args.cgeom[DY] = args.bgeom[DY];
    args.cgeom[DZ] = args.bgeom[DZ];
  } else {
    parse_dims(cg, &args.cgeom, &args.cgeom_size);
  }

  if (NDIMS != args.bgeom_size) {
    FATAL_ERR("Found %zu dims (expected %d).\n", args.bgeom_size, NDIMS);
  }

  if (NULL != args.cgeom) {
    if (NDIMS != args.cgeom_size) {
      FATAL_ERR("Found %zu dims (expected %d).\n", args.cgeom_size, NDIMS);
    }
  }

  DEBUG_MESSAGE("dgeom (%zu:%zu:%zu:%zu)\n", args.dgeom[DT], args.dgeom[DX], args.dgeom[DY], args.dgeom[DZ]);
  DEBUG_MESSAGE("bgeom (%zu:%zu:%zu:%zu)\n", args.bgeom[DT], args.bgeom[DX], args.bgeom[DY], args.bgeom[DZ]);
  if (NULL != args.cgeom) {
    DEBUG_MESSAGE("cgeom (%zu:%zu:%zu:%zu)\n", args.cgeom[DT], args.cgeom[DX], args.cgeom[DY], args.cgeom[DZ]);
  }
  DEBUG_MESSAGE("(nn %zu, ppn %zu)\n", args.procs.nn, args.procs.ppn);
  DEBUG_MESSAGE("test filename %s\n", args.testfn);

  if (args.dgeom[DX] % args.procs.nn != 0) {
    FATAL_ERR("x must be a multiple of number of nodes.\n");
  }

  if (args.dgeom[DY] % args.procs.ppn != 0) {
    FATAL_ERR("y must be a multiple of number of processes.\n");
  }

  if (NULL != args.cgeom) {
    if (args.dgeom[DT] % args.cgeom[DT] != 0) {
      FATAL_ERR("Time range must be a multiple of time slice (range=%zu; slice=%zu)\n", args.dgeom[DT], args.cgeom[DT]);
    }
  }

  int nranks = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);

  if (nranks != args.procs.nn * args.procs.ppn) {
    FATAL_ERR("Bad environment: np != nn * ppn; np(size of MPI_COMM_WORLD)=%d, nodes(nn)=%zu, ppn(procs per node)=%zu\n",
    nranks, args.procs.nn, args.procs.ppn);
  }


  if ((args.read_test == false) & (args.write_test == false) & (args.verify == false)) {
    args.write_test = true;
  }

  benchmark_t wbm;
  benchmark_init(&wbm);

  int header_printed = 0;

  benchmark_t rbm;
  benchmark_init(&rbm);
  if (args.write_test || args.verify) {
    benchmark_setup(&wbm, args.procs, NDIMS, args.dgeom, args.bgeom, args.cgeom, args.testfn, IO_MODE_WRITE, args.par_access, args.is_unlimited, args.fill_value);
    if (rank == 0) {
      print_header(&wbm);
      header_printed = 1;
    }
  }
  if (args.write_test) {
    benchmark_run(&wbm, NULL);
    report_t report;
    report_init(&report);
    report_setup(&report, &wbm);
    report_print(&report, args.report_type);
    report_destroy(&report);
  }
  if (args.read_test) {
    int ret;
    benchmark_setup(&rbm, args.procs, NDIMS, args.dgeom, args.bgeom, args.cgeom, args.testfn, IO_MODE_READ, args.par_access, args.is_unlimited, 0);
    if (rank == 0 && !header_printed) {
      print_header(&rbm);
      header_printed = 1;
    }
    ret = benchmark_run(&rbm, args.verify ? wbm.block : NULL);
    report_t report;
    report_init(&report);
    report_setup(&report, &rbm);
    report_print(&report, args.report_type);
    report_destroy(&report);

  } else if (args.verify) {
    int ret;
    benchmark_setup(&rbm, args.procs, NDIMS, args.dgeom, args.bgeom, args.cgeom, args.testfn, IO_MODE_READ, args.par_access, args.is_unlimited, 0);
    if (rank == 0 && !header_printed) {
      print_header(&rbm);
      header_printed = 1;
    }
    ret = benchmark_run(&rbm, wbm.block);
    if (args.verify) {
      if (ret) {
        printf("TEST PASSED [%u]\n", wbm.rank);
      } else {
        printf("TEST FAILED [%u]\n", wbm.rank);
      }
    }
  }


  MPI_Finalize();
  benchmark_destroy(&wbm);
  benchmark_destroy(&rbm);
  free(args.dgeom);
  free(args.bgeom);
  free(args.cgeom);
  free(args.testfn);
  return 0;
}
