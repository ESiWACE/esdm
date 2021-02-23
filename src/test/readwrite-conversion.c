/*
 * This test uses the ESDM high-level API to actually write a contiuous ND subset of a data set
 */


#include <esdm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <esdm-internal.h>

#define HEIGHT 10
#define WIDTH  20
#define COUNT  10

int verify_data(float *a, double *b) {
  int mismatches = 0;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = y * WIDTH + x;

      if (a[idx] != (float) b[idx]) {
        mismatches++;
        printf("idx=%10d, x=%10d, y=%10d should be %f but is %f\n", idx, x, y, (double) a[idx], b[idx]);
      }
    }
  }

  return mismatches;
}

/** Calculate bandwidth in bytes / sec. Print KiB/s */
static uint64_t calc_bw(const char *tag, uint64_t bytes, struct timeval tv_begin)
{
  struct timeval tv_end;
  uint64_t ms_total;
  uint64_t bw;

  gettimeofday(&tv_end, NULL);
  ms_total = tv_end.tv_sec * 1000000 + tv_end.tv_usec - tv_begin.tv_sec * 1000000 - tv_begin.tv_usec;

  if(ms_total == 0) return -1;
  bw = bytes * 1000000 / ms_total;

  fprintf(stderr, "%s %8"PRIu64" KiB BW %8"PRIu64" KiB/s\n", tag, bytes >> 10, bw >> 10);
  return bw;
}

int main(int argc, char const *argv[]) {
  // prepare data
  float *buf_w = ea_checked_malloc(HEIGHT * WIDTH * sizeof(float));
  double *buf_r = ea_checked_malloc(HEIGHT * WIDTH * sizeof(double));
  struct timeval tv_origin;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = y * WIDTH + x;
      buf_w[idx] = idx;
    }
  }

  // Interaction with ESDM
  esdm_status ret;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  // define dataspace
  esdm_simple_dspace_t dataspace = esdm_dataspace_2d(HEIGHT, WIDTH*COUNT, SMD_DTYPE_FLOAT);
  eassert(dataspace.ptr);

  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_create(container, "mydataset", dataspace.ptr, &dataset);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  gettimeofday(&tv_origin, NULL);
  for (int n = 0 ; n < COUNT; n++) {
    struct timeval tv_begin;
    // define subspace
    esdm_simple_dspace_t subspace = esdm_dataspace_2do(0, HEIGHT, n*WIDTH, WIDTH, SMD_DTYPE_FLOAT);
    eassert(subspace.ptr);

    gettimeofday(&tv_begin, NULL);
    // Write the data to the dataset
    ret = esdm_write(dataset, buf_w, subspace.ptr);
    eassert(ret == ESDM_SUCCESS);

    ret = esdm_dataset_commit(dataset);
    eassert(ret == ESDM_SUCCESS);
    calc_bw("Write", HEIGHT * WIDTH * 4, tv_begin);

    memset(buf_r, 0, HEIGHT * WIDTH * sizeof(double));
    // Read the data to the dataset
    gettimeofday(&tv_begin, NULL);
    
    esdm_simple_dspace_t subspace_r = esdm_dataspace_2do(0, HEIGHT, n*WIDTH, WIDTH, SMD_DTYPE_DOUBLE);    
    ret = esdm_read(dataset, buf_r, subspace_r.ptr);
    eassert(ret == ESDM_SUCCESS);
    calc_bw("Read ", HEIGHT * WIDTH * 8, tv_begin);

    // TODO: write subset
    // TODO: read subset -> subspace reconstruction

    // verify data and fail test if mismatches are found
    int mismatches = verify_data(buf_w, buf_r);
    printf("Mismatches: %d\n", mismatches);
    if (mismatches > 0) {
      printf("FAILED\n");
    } else {
      printf("OK\n");
    }
    eassert(mismatches == 0);
  }
  calc_bw("Total", HEIGHT * WIDTH * COUNT * 2 * 8, tv_origin);

  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  // clean up
  free(buf_w);
  free(buf_r);

  return 0;
}
