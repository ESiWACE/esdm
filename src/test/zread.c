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
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_container_open("mycontainer", ESDM_MODE_FLAG_READ, NULL));
  eassert(esdm_container_open("", ESDM_MODE_FLAG_READ, &container) == ESDM_INVALID_ARGUMENT_ERROR);
  status = esdm_container_open("mycontainer", ESDM_MODE_FLAG_READ, &container);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_dataset_open(NULL, "mydataset", ESDM_MODE_FLAG_READ, &dataset));
  eassert_crash(esdm_dataset_open(container, "mydataset",ESDM_MODE_FLAG_READ, NULL));
  eassert(esdm_dataset_open(container, "",ESDM_MODE_FLAG_READ, &dataset) == ESDM_INVALID_ARGUMENT_ERROR);
  status = esdm_dataset_open(container, "mydataset",ESDM_MODE_FLAG_READ, &dataset);
  eassert(status == ESDM_SUCCESS);

  //failing input tests are in write.c
  esdm_dataspace_t *space = esdm_dataspace_create_2d(0, 10, 0, 20, SMD_DTYPE_UINT64);
  eassert(space);

  eassert_crash(esdm_read(NULL, buf_r, space));
  eassert_crash(esdm_read(dataset, NULL, space));
  eassert_crash(esdm_read(dataset, buf_r, NULL));
  status = esdm_read(dataset, buf_r, space);
  eassert(status == ESDM_SUCCESS);

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);

  // verify data and fail test if mismatches are found
  int mismatches = verify_data(buf_w, buf_r);
  printf("Mismatches: %d\n", mismatches);
  if (mismatches > 0) {
    printf("FAILED\n");
  } else {
    printf("OK\n");
  }
  eassert(mismatches == 0);

  // clean up
  free(buf_w);
  free(buf_r);

  return 0;
}
