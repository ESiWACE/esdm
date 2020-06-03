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
  esdm_dataspace_t *dataspace;
  int64_t offset[2] = {0, 0}, size[2] = {10, 20};

  status = esdm_dataspace_create(2, size, SMD_DTYPE_UINT64, &dataspace);
  eassert(status == ESDM_SUCCESS);
  status = esdm_container_create("mycontainer", 1, &container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  esdm_wstream_uint64_t stream;
  esdm_wstream_start(&stream, dataset, 2, offset, size);

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      esdm_wstream_pack(stream, (y)*10 + x + 1);
    }
  }
  esdm_wstream_commit(stream);
  status = esdm_container_commit(container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);
  printf("\nOK\n");

  return 0;
}
