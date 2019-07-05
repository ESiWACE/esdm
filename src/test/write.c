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
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  esdm_status status = esdm_init();
  assert(status == ESDM_SUCCESS);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  assert(status == ESDM_SUCCESS);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  assert(status == ESDM_SUCCESS);

  // define dataspace
  int64_t bounds[] = {10, 20};
  esdm_dataspace_t *dataspace;

  assert_crash(esdm_dataspace_create(0xc000000000000000ll, bounds, SMD_DTYPE_UINT64, &dataspace));
  assert_crash(esdm_dataspace_create(2, NULL, SMD_DTYPE_UINT64, &dataspace));
  assert_crash(esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, NULL));
  status = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);
  assert(status == ESDM_SUCCESS);

  assert_crash(esdm_container_create(NULL, &container));
  assert_crash(esdm_container_create("", &container));
  assert_crash(esdm_container_create("mycontainer", NULL));
  status = esdm_container_create("mycontainer", &container);
  assert(status == ESDM_SUCCESS);

  assert_crash(esdm_dataset_create(NULL, "mydataset", dataspace, &dataset));
  assert_crash(esdm_dataset_create(container, NULL, dataspace, &dataset));
  assert_crash(esdm_dataset_create(container, "", dataspace, &dataset));
  assert_crash(esdm_dataset_create(container, "mydataset", NULL, &dataset));
  assert_crash(esdm_dataset_create(container, "mydataset", dataspace, NULL));
  status = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  assert(status == ESDM_SUCCESS);

  assert_crash(esdm_container_commit(NULL));
  status = esdm_container_commit(container);
  assert(status == ESDM_SUCCESS);

  assert_crash(esdm_dataset_commit(NULL));
  status = esdm_dataset_commit(dataset);
  assert(status == ESDM_SUCCESS);

  // define subspace
  int64_t size[] = {10, 20};
  int64_t offset[] = {0, 0};
  esdm_dataspace_t *subspace;

  assert_crash(esdm_dataspace_subspace(NULL, 2, size, offset, &subspace));
  assert_crash(esdm_dataspace_subspace(dataspace, 2, NULL, offset, &subspace));
  assert_crash(esdm_dataspace_subspace(dataspace, 2, size, NULL, &subspace));
  assert_crash(esdm_dataspace_subspace(dataspace, 2, size, offset, NULL));
  assert(esdm_dataspace_subspace(dataspace, 1, size, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){-1, 1}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){11, 1}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){1, -1}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){10, 21}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){-1, 0}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){1, 0}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){0, -1}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  assert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){0, 1}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  status = esdm_dataspace_subspace(dataspace, 2, size, offset, &subspace);
  assert(status == ESDM_SUCCESS);

  // Write the data to the dataset
  status = esdm_write(dataset, buf_w, subspace);
  assert(status == ESDM_SUCCESS);

  esdm_container_commit(container);
  esdm_dataset_commit(dataset);

  status = esdm_finalize();
  assert(status == ESDM_SUCCESS);

  // clean up
  free(buf_w);

  return 0;
}
