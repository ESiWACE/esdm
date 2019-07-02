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

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t *buf_w = (uint64_t *)malloc(10 * 20 * sizeof(uint64_t));

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      buf_w[y * 10 + x] = (y)*10 + x + 1;
    }
  }

  // Interaction with ESDM
  esdm_status ret;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  ret = esdm_init();
  assert(ret == ESDM_SUCCESS);

  //ret = esdm_create("mytextfile", ESDM_CREATE, &container, &dataset);
  //assert(ret == ESDM_SUCCESS);

  //esdm_open("mycontainer/mydataset", ESDM_CREATE);

  // POSIX pwrite/pread interfaces for comparison
  //ssize_t pread(int fd, void *buf, size_t count, off_t offset);
  //ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

  // define dataspace
  int64_t bounds[] = {10, 20};
  esdm_dataspace_t *dataspace;

  assert_crash(esdm_dataspace_create(0xc000000000000000ll, bounds, SMD_DTYPE_UINT64, &dataspace));
  assert_crash(esdm_dataspace_create(2, NULL, SMD_DTYPE_UINT64, &dataspace));
  assert_crash(esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, NULL));
  esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);

  esdm_container_create("mycontainer", &container);
  esdm_dataset_create(container, "mydataset", dataspace, &dataset);

  esdm_container_commit(container);
  esdm_dataset_commit(dataset);

  // define subspace
  int64_t size[] = {10, 20};
  int64_t offset[] = {0, 0};
  esdm_dataspace_t *subspace;

  esdm_dataspace_subspace(dataspace, 2, size, offset, &subspace);

  // Write the data to the dataset
  ret = esdm_write(dataset, buf_w, subspace);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_finalize();
  assert(ret == ESDM_SUCCESS);

  // clean up
  free(buf_w);

  return 0;
}
