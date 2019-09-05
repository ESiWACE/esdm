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

typedef struct{
  size_t mismatches;
  size_t checked;
  uint64_t * expected_buf;
} my_user_data_t;

typedef struct{
  size_t mismatches;
  size_t checked;
} my_tmp_result_t;


static void* stream_func(esdm_dataspace_t *space, void * buff, void * user_ptr, void* esdm_fill_value){
  my_user_data_t * up = (my_user_data_t*) user_ptr;
  uint64_t *a = (uint64_t *) buff;
  uint64_t *b = up->expected_buf;
  size_t mismatches = 0;

  int64_t const* s = esdm_dataspace_get_size(space);
  int64_t const* o = esdm_dataspace_get_offset(space);

  int idx;
  int x_end = s[0] + o[0];
  int y_end = s[1] + o[1];
  for (int x = o[0]; x < x_end; x++) {
    for (int y = o[1]; y < y_end; y++) {
      idx = y * 10 + x;
      if (a[idx] != b[idx]) {
        mismatches++;
      }
    }
  }

  my_tmp_result_t * tmp = malloc(sizeof(my_tmp_result_t));
  tmp->mismatches = mismatches;
  tmp->checked = s[0] * s[1];
  return tmp;
}

static void reduce_func(esdm_dataspace_t *space, void * user_ptr, void * stream_func_out){
  my_user_data_t * up = (my_user_data_t*) user_ptr;
  my_tmp_result_t * tmp = (my_tmp_result_t*) stream_func_out;
  up->mismatches += tmp->mismatches;
  up->checked += tmp->checked;
  free(stream_func_out);
}

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t *buf_w = (uint64_t *)malloc(10 * 20 * sizeof(uint64_t));

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
  status = esdm_container_open("mycontainer", ESDM_MODE_FLAG_READ, &container);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_open(container, "mydataset",ESDM_MODE_FLAG_READ, &dataset);
  eassert(status == ESDM_SUCCESS);

  int64_t size[] = {10, 20};
  esdm_dataspace_t *space;

  //failing input tests are in write.c
  status = esdm_dataspace_create(2, size, SMD_DTYPE_UINT64, &space);
  eassert(status == ESDM_SUCCESS);

  my_user_data_t user_data = {0, 0, buf_w};
  status = esdm_read_stream(dataset, space, & user_data, stream_func, reduce_func);
  eassert(status == ESDM_SUCCESS);

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);

  printf("Checked: %zu mismatches: %zu\n", user_data.checked, user_data.mismatches);
  if (user_data.mismatches > 0) {
    printf("FAILED\n");
  } else {
    printf("OK\n");
  }
  eassert(user_data.mismatches == 0);
  eassert(user_data.checked == 200);

  // clean up
  free(buf_w);

  return 0;
}
