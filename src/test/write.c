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

#include <esdm-internal.h>
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
  eassert(status == ESDM_SUCCESS);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(status == ESDM_SUCCESS);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(status == ESDM_SUCCESS);

  // define dataspace
  int64_t bounds[] = {10, 20};
  esdm_dataspace_t *dataspace;

  eassert_crash(esdm_dataspace_create(0xc000000000000000ll, bounds, SMD_DTYPE_UINT64, &dataspace));
  eassert_crash(esdm_dataspace_create(2, NULL, SMD_DTYPE_UINT64, &dataspace));
  eassert_crash(esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, NULL));
  status = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_container_create(NULL, 1, &container));
  eassert_crash(esdm_container_create("", 1, &container));
  eassert_crash(esdm_container_create("mycontainer", 1, NULL));
  status = esdm_container_create("/po/test", 1, &container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_container_close(container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_container_create("po/test/", 1, &container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_container_close(container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_container_create("mycontainer", 1, &container);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_dataset_create(NULL, "mydataset", dataspace, &dataset));
  eassert_crash(esdm_dataset_create(container, NULL, dataspace, &dataset));
  eassert_crash(esdm_dataset_create(container, "", dataspace, &dataset));
  eassert_crash(esdm_dataset_create(container, "mydataset", NULL, &dataset));
  eassert_crash(esdm_dataset_create(container, "mydataset", dataspace, NULL));
  status = esdm_dataset_create(container, "/mydataset", dataspace, &dataset);
  eassert(status == ESDM_ERROR);
  status = esdm_dataset_create(container, "öüö&asas", dataspace, &dataset);
  eassert(status == ESDM_ERROR);
  status = esdm_dataset_create(container, "test/", dataspace, &dataset);
  eassert(status == ESDM_ERROR);
  status = esdm_dataset_create(container, "test//test", dataspace, &dataset);
  eassert(status == ESDM_ERROR);
  status = esdm_dataset_create(container, "/test/test", dataspace, &dataset);
  eassert(status == ESDM_ERROR);
  status = esdm_dataset_create(container, "test/test", dataspace, &dataset);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_close(dataset);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_container_commit(NULL));
  status = esdm_container_commit(container);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_dataset_commit(NULL));
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  // define subspace
  int64_t size[] = {10, 20};
  int64_t offset[] = {0, 0};
  esdm_dataspace_t *subspace;

  eassert_crash(esdm_dataspace_subspace(NULL, 2, size, offset, &subspace));
  eassert_crash(esdm_dataspace_subspace(dataspace, 2, NULL, offset, &subspace));
  eassert_crash(esdm_dataspace_subspace(dataspace, 2, size, NULL, &subspace));
  eassert_crash(esdm_dataspace_subspace(dataspace, 2, size, offset, NULL));
  //eassert(esdm_dataspace_subspace(dataspace, 1, size, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){-1, 1}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){11, 1}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){1, -1}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, (int64_t[2]){10, 21}, offset, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){-1, 0}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){1, 0}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){0, -1}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  eassert(esdm_dataspace_subspace(dataspace, 2, size, (int64_t[2]){0, 1}, &subspace) == ESDM_INVALID_ARGUMENT_ERROR);
  status = esdm_dataspace_subspace(dataspace, 2, size, offset, &subspace);
  eassert(status == ESDM_SUCCESS);

  // Write the data to the dataset
  printf("Write 0\n");
  eassert_crash(esdm_write(NULL, buf_w, subspace));
  printf("Write 1\n");
  eassert_crash(esdm_write(dataset, NULL, subspace));
  printf("Write 2\n");
  eassert_crash(esdm_write(dataset, buf_w, NULL));
  printf("Write 3\n");
  status = esdm_write(dataset, buf_w, subspace);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_container_commit(NULL));
  status = esdm_container_commit(container);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_dataset_commit(NULL));
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_dataset_close(NULL));
  status = esdm_dataset_close(dataset);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_container_close(NULL));
  status = esdm_container_close(container);
  eassert(status == ESDM_SUCCESS);

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);

  // clean up
  free(buf_w);
  status = esdm_dataspace_destroy(dataspace);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataspace_destroy(subspace);
  eassert(status == ESDM_SUCCESS);

  printf("\nOK\n");

  return 0;
}
