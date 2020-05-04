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

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t *buf_w = (uint64_t *)malloc(10 * 20 * sizeof(uint64_t));

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      buf_w[y * 10 + x] = (y)*10 + x + 1;
    }
  }

  // Interaction with ESDM
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  // define dataspace
  esdm_simple_dspace_t dataspace = esdm_dataspace_2d(10, 20, SMD_DTYPE_UINT64);
  eassert(dataspace.ptr);

  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_create(container, "mydataset", dataspace.ptr, &dataset);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  // define subspace
  int64_t size[] = {10, 20};
  int64_t offset[] = {0, 0};
  esdm_dataspace_t *subspace;

  ret = esdm_dataspace_subspace(dataspace.ptr, 2, size, offset, &subspace);
  eassert(ret == ESDM_SUCCESS);

  // Write the data to the dataset
  ret = esdm_write(dataset, buf_w, subspace);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  // clean up
  free(buf_w);

  printf("\nOK\n");

  return 0;
}
