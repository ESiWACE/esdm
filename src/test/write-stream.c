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

//TODO: Expand this into a benchmark.
int main(int argc, char const *argv[]) {
  // Interaction with ESDM
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  esdm_loglevel(ESDM_LOGLEVEL_WARNING); //stop the esdm_mkfs() call from spamming us with infos about deleted objects
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

  for (int x = 0; x < size[0]; x++) {
    for (int y = 0; y < size[1]; y++) {
      esdm_wstream_pack(stream, (x)*size[1] + y);
    }
  }
  esdm_wstream_commit(stream);
  status = esdm_container_commit(container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  // read back the data and check that it's ok
  uint64_t (*data)[size[1]] = malloc(size[0]*sizeof*data);
  status = esdm_read(dataset, data, dataspace);
  eassert(status == ESDM_SUCCESS);
  printf("data after read-back:\n");
  for (int x = 0; x < size[0]; x++) {
    for (int y = 0; y < size[1]; y++) {
      printf("%s%"PRId64, y ? ", " : "\t", data[x][y]);
    }
    printf("\n");
  }
  for (int x = 0; x < size[0]; x++) {
    for (int y = 0; y < size[1]; y++) {
      eassert(data[x][y] == (x)*size[1] + y);
    }
  }

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);
  printf("\nOK\n");

  return 0;
}
