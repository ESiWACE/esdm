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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>
#include <esdm-internal.h>
#include <esdm-mpi.h>
#include "util/test_util.h"


int mpi_size;
int mpi_rank;
int run_read = 0;
int run_write = 0;
long timesteps = 0;
int cycleBlock = 0;
int64_t size;
int64_t volume;
int64_t volume_all;

void runWrite(uint64_t * buf_w, int64_t * dim, int64_t * offset){
  esdm_status ret;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;
  // define dataspace
  int64_t bounds[] = {timesteps, size, size};
  esdm_dataspace_t *dataspace;

  esdm_mpi_container_create(MPI_COMM_WORLD, "mycontainer", &container);
  esdm_dataspace_create(3, bounds, SMD_DTYPE_UINT64, &dataspace);
  esdm_mpi_dataset_create(MPI_COMM_WORLD, container, "mydataset", dataspace, &dataset);

  timer t;
  double time;

  MPI_Barrier(MPI_COMM_WORLD);
  start_timer(&t);
  // Write the data to the dataset
  for (int t = 0; t < timesteps; t++) {
    offset[0] = t;
    esdm_dataspace_t *subspace;

    esdm_dataspace_subspace(dataspace, 3, dim, offset, &subspace);
    buf_w[0] = t;
    ret = esdm_write(dataset, buf_w, subspace);
    assert(ret == ESDM_SUCCESS);
  }

  // commit the changes to data to the metadata
  esdm_mpi_container_commit(MPI_COMM_WORLD, container);
  esdm_mpi_dataset_commit(MPI_COMM_WORLD, dataset);

  MPI_Barrier(MPI_COMM_WORLD);
  time = stop_timer(t);
  double total_time;
  MPI_Reduce((void *)&time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if (mpi_rank == 0) {
    printf("Write: %.3fs %.3f MiB/s size:%.0f MiB\n", total_time, volume_all / total_time / 1024.0 / 1024, volume_all / 1024.0 / 1024);
  }

  ret = esdm_dataset_destroy(dataset);
  assert(ret == ESDM_SUCCESS);
  ret = esdm_container_destroy(container);
  assert(ret == ESDM_SUCCESS);
}

void runRead(uint64_t * buf_w, int64_t * dim, int64_t * offset){
  esdm_status ret;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;
  // define dataspace
  esdm_dataspace_t *dataspace;

  ret = esdm_mpi_container_open(MPI_COMM_WORLD, "mycontainer", &container);
  assert(ret == ESDM_SUCCESS);
  ret = esdm_mpi_dataset_open(MPI_COMM_WORLD, container, "mydataset", &dataset);
  assert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_get_dataspace(dataset, & dataspace);
  assert(ret == ESDM_SUCCESS);

  timer t;
  double time;

  int mismatches = 0;
  MPI_Barrier(MPI_COMM_WORLD);
  start_timer(&t);
  // Read the data to the dataset
  for (int t = 0; t < timesteps; t++) {
    offset[0] = t;
    uint64_t *buf_r = (uint64_t *) malloc(volume);
    buf_r[0] = -1241;
    assert(buf_r != NULL);
    esdm_dataspace_t *subspace;
    esdm_dataspace_subspace(dataspace, 3, dim, offset, &subspace);

    ret = esdm_read(dataset, buf_r, subspace);
    assert(ret == ESDM_SUCCESS);

    // verify data and fail test if mismatches are found
    buf_w[0] = t;
    for (int y = 0; y < dim[1]; y++) {
      for (int x = 0; x < dim[2]; x++) {
        long idx = y * size + x;
        if (buf_r[idx] != buf_w[idx]) {
          mismatches++;
        }
      }
    }
    free(buf_r);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  time = stop_timer(t);

  int mismatches_sum = 0;
  MPI_Reduce(&mismatches, &mismatches_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  double total_time;
  MPI_Reduce((void *)&time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if (mpi_rank == 0) {
    if (mismatches_sum > 0) {
      printf("FAILED\n");
      printf("Mismatches: %d\n", mismatches_sum);
    } else {
      printf("OK\n");
    }
    printf("Read: %.3fs %.3f MiB/s size:%.0f MiB\n", total_time, volume_all / total_time / 1024.0 / 1024, volume_all / 1024.0 / 1024);
  }
  ret = esdm_dataset_destroy(dataset);
  assert(ret == ESDM_SUCCESS);
  ret = esdm_container_destroy(container);
  assert(ret == ESDM_SUCCESS);
}

int main(int argc, char *argv[]) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  int64_t _size;
  char *config_file;
  char *default_args[] = {argv[0], "1024", "_esdm.conf", "B", "10"};
  if (argc == 1) {
    argc = 5;
    argv = default_args;
  }
  if (argc != 5) {
    printf("Syntax: %s [SIZE] [CONFIG] [B|R|W][C] [TIMESTEPS]", argv[0]);
    printf("\t SIZE specifies one dimension of a 2D field\n");
    exit(1);
  }

  _size = atol(argv[1]);
  config_file = argv[2];
  switch (argv[3][0]) {
    case ('R'): {
      run_read = 1;
      break;
    }
    case ('W'): {
      run_write = 1;
      break;
    }
    case ('B'): {
      run_read = 1;
      run_write = 1;
      break;
    }
    default: {
      printf("Unknown setting for argument: %s expected [R|W|B]\n", argv[3]);
      exit(1);
    }
  }
  cycleBlock = argv[3][1] == 'C';
  timesteps = atol(argv[4]);

  size = _size;

  if (mpi_rank == 0)
    printf("Running with %ld timesteps and 2D slice of %ld*%ld (cycle: %d)\n", timesteps, size, size, cycleBlock);

  if (size / mpi_size == 0) {
    printf("Error, size < number of ranks!\n");
    exit(1);
  }

  int pPerNode = esdm_mpi_get_tasks_per_node();
  int tmp_rank = (mpi_rank + (cycleBlock * pPerNode)) % mpi_size;
  int64_t dim[] = {1, size / mpi_size + (tmp_rank < (size % mpi_size) ? 1 : 0), size};
  int64_t offset[] = {0, size / mpi_size * tmp_rank + (tmp_rank < (size % mpi_size) ? tmp_rank : size % mpi_size), 0};

  volume = dim[1] * dim[2] * sizeof(uint64_t);
  volume_all = timesteps * size * size * sizeof(uint64_t);

  if (!volume_all) {
    printf("Error: no data!\n");
    exit(1);
  }

  // prepare data
  uint64_t *buf_w = (uint64_t *)malloc(volume);
  assert(buf_w != NULL);
  long x, y;
  for (y = 0; y < dim[1]; y++) {
    for (x = 0; x < dim[2]; x++) {
      long idx = y * size + x;
      buf_w[idx] = (y+offset[1]) * size + x + offset[2] + 1 + mpi_rank;
    }
  }

  esdm_status ret;

  esdm_mpi_init();
  esdm_mpi_distribute_config_file(config_file);

  ret = esdm_init();
  assert(ret == ESDM_SUCCESS);

  if (run_write) {
    runWrite(buf_w, dim, offset);
  }

  if (run_read) {
    runRead(buf_w, dim, offset);
  }

  esdm_mpi_finalize();

  // clean up
  free(buf_w);

  MPI_Finalize();

  return 0;
}
