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

#include <test/util/test_util.h>

#include <assert.h>
#include <esdm.h>
#include <stdio.h>
#include <stdlib.h>

int verify_data(uint64_t *a, uint64_t *b) {
  int mismatches = 0;
  int idx;

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      idx = y * 10 + x;

      if (a[idx] != b[idx]) {
        mismatches++;
        //printf("idx=%04d, x=%04d, y=%04d should be %10ld but is %10ld\n", idx, x, y, a[idx], b[idx]);
      }
    }
  }

  return mismatches;
}

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t *buf_w = (uint64_t *)malloc(10 * 20 * sizeof(uint64_t));
  uint64_t *buf_r = (uint64_t *)malloc(10 * 20 * sizeof(uint64_t));
  memset(buf_r, -1, 10 * 20 * sizeof(uint64_t));

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      buf_w[y * 10 + x] = (y)*10 + x + 1;
    }
  }

  // Interaction with ESDM
  esdm_status status;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  status = esdm_init();
  assert(status == ESDM_SUCCESS);

  assert_crash(esdm_container_open("mycontainer", NULL));
  assert(esdm_container_open("", &container) == ESDM_INVALID_ARGUMENT_ERROR);
  status = esdm_container_open("mycontainer", &container);
  assert(status == ESDM_SUCCESS);

  assert_crash(esdm_dataset_open(NULL, "mydataset", &dataset));
  assert_crash(esdm_dataset_open(container, "mydataset", NULL));
  assert(esdm_dataset_open(container, "", &dataset) == ESDM_INVALID_ARGUMENT_ERROR);
  status = esdm_dataset_open(container, "mydataset", &dataset);
  assert(status == ESDM_SUCCESS);

  int64_t size[] = {10, 20};
  esdm_dataspace_t *space;

  esdm_dataspace_create(2, size, SMD_DTYPE_UINT64, &space);

  status = esdm_read(dataset, buf_r, space);
  assert(status == ESDM_SUCCESS);

  status = esdm_finalize();
  assert(status == ESDM_SUCCESS);

  // verify data and fail test if mismatches are found
  int mismatches = verify_data(buf_w, buf_r);
  printf("Mismatches: %d\n", mismatches);
  if (mismatches > 0) {
    printf("FAILED\n");
  } else {
    printf("OK\n");
  }
  assert(mismatches == 0);

  // clean up
  free(buf_w);
  free(buf_r);

  return 0;
}
