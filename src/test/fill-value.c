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
 * This test tests the unlimited dimension features of ESDM.
 */

#include <stdio.h>
#include <stdlib.h>

#include <test/util/test_util.h>
#include <esdm.h>

int main(int argc, char const *argv[]) {
  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  int dims = 1;
  int64_t bounds[] = {200};

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  esdm_dataspace_t *dataspace;

  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataspace_create(dims, bounds, SMD_DTYPE_INT64, &dataspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  eassert(ret == ESDM_SUCCESS);

  uint64_t fill_value = 64;
  uint64_t fill_value_read = 222;
  ret = esdm_dataset_set_fill_value(dataset, & fill_value);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_get_fill_value(dataset, & fill_value_read);
  eassert(ret == ESDM_SUCCESS);
  eassert(fill_value_read == fill_value);

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_container_open("mycontainer", &container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_open(container, "mydataset", &dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_get_fill_value(dataset, & fill_value_read);
  eassert(ret == ESDM_SUCCESS);
  eassert(fill_value_read == fill_value);


  printf("%lu %lu\n", fill_value, fill_value_read);

  uint64_t buf_w[200];
  ret = esdm_read(dataset, buf_w, dataspace);
  eassert(ret == ESDM_SUCCESS);

  // TODO
  for(int i=0; i < 200; i++){
    //eassert(buf_w[i] == fill_value);
  }

  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");

  return 0;
}
