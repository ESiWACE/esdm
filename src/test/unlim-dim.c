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


void test(char const * str, int dims, int64_t bounds_dset[], int64_t bounds_data[], int64_t offset_data[]){
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;
  esdm_status ret;

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  printf("\nRunning test: %s\n", str);
  esdm_dataspace_t *dataspace;

  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataspace_create(dims, bounds_dset, SMD_DTYPE_UINT64, &dataspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  eassert(ret == ESDM_SUCCESS);

  // define subspace
  esdm_dataspace_t *subspace;
  ret = esdm_dataspace_subspace(dataspace, dims, bounds_data, offset_data, &subspace);
  eassert(ret == ESDM_SUCCESS);

  // Write the data to the dataset

  uint64_t * buf_w;
  etest_gen_buffer(dims, bounds_data, & buf_w);
  ret = esdm_write(dataset, buf_w, subspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  etest_memset_buffer(dims, bounds_data, buf_w);

  ret = esdm_read(dataset, buf_w, subspace);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_dataspace_destroy(dataspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataspace_destroy(subspace);
  eassert(ret == ESDM_SUCCESS);

  ret = etest_verify_buffer(dims, bounds_data, buf_w);
  eassert(ret == 0);

  int64_t const * size = esdm_dataset_get_actual_size(dataset);
  for(int i=0; i < dims; i++){
    printf("%ld\n", size[i]);
    if(bounds_dset[i] == 0){
      eassert(size[i] == bounds_data[i] + offset_data[i]);
    }else{
      eassert(size[i] == bounds_dset[i]);
    }
  }

  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_container_open("mycontainer", ESDM_MODE_FLAG_READ, &container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_open(container, "mydataset", ESDM_MODE_FLAG_READ, &dataset);
  eassert(ret == ESDM_SUCCESS);
  size = esdm_dataset_get_actual_size(dataset);
  for(int i=0; i < dims; i++){
    printf("%ld\n", size[i]);
    if(bounds_dset[i] == 0){
      eassert(size[i] == bounds_data[i] + offset_data[i]);
    }else{
      eassert(size[i] == bounds_dset[i]);
    }
  }
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  // clean up
  free(buf_w);
}

int main(int argc, char const *argv[]) {
  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  test("3", 1, (int64_t[]){0}, (int64_t[]){4}, (int64_t[]){0});
  test("2", 2, (int64_t[]){20, 20}, (int64_t[]){10,10}, (int64_t[]){10,10});
  test("4", 1, (int64_t[]){0}, (int64_t[]){3}, (int64_t[]){5});
  test("5", 2, (int64_t[]){0,20}, (int64_t[]){3,20}, (int64_t[]){5,0});

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");

  return 0;
}
