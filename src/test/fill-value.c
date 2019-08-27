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

  int64_t bounds[] = {100, 100};
  const int dims = sizeof(bounds)/sizeof(*bounds);

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

  //fake some data
  int64_t subspaceSize[] = {50, 50};
  eassert(dims == sizeof(subspaceSize)/sizeof(*subspaceSize));
  int64_t subspaceOffset[] = {25, 25};
  eassert(dims == sizeof(subspaceOffset)/sizeof(*subspaceOffset));
  uint64_t writeBuffer[subspaceSize[0]][subspaceSize[1]];
  for(int y = 0; y < subspaceSize[0]; y++){
    for(int x = 0; x < subspaceSize[1]; x++){
      writeBuffer[y][x] = (y+subspaceOffset[0])*bounds[1] + x+subspaceOffset[1];
    }
  }

  //write a subregion
  esdm_dataspace_t* subspace;
  ret = esdm_dataspace_subspace(dataspace, dims, subspaceSize, subspaceOffset, &subspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_write(dataset, writeBuffer, subspace);
  eassert(ret == ESDM_SUCCESS);

  //commit and close stuff
  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  //reopen the dataset and play with changing the fill value
  ret = esdm_container_open("mycontainer", ESDM_MODE_FLAG_READ, &container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_open(container, "mydataset", ESDM_MODE_FLAG_READ, &dataset);
  eassert(ret == ESDM_SUCCESS);
  fill_value_read = 5417;
  ret = esdm_dataset_get_fill_value(dataset, & fill_value_read);
  eassert(ret == ESDM_SUCCESS);
  eassert(fill_value_read == fill_value);
  fill_value = 42;
  ret = esdm_dataset_set_fill_value(dataset, & fill_value);
  eassert(ret == ESDM_SUCCESS);
  fill_value_read = 5417;
  ret = esdm_dataset_get_fill_value(dataset, & fill_value_read);
  eassert(ret == ESDM_SUCCESS);
  eassert(fill_value_read == fill_value);

  //read the entire region back
  uint64_t readBuffer[bounds[0]][bounds[1]];
  ret = esdm_read(dataset, readBuffer, dataspace);
  eassert(ret == ESDM_SUCCESS);

  //check that the result is actually what we expect (linear index within subspace, updated fill_value everywhere else)
  for(int y=0; y < bounds[0]; y++){
    for(int x=0; x < bounds[1]; x++){
      uint64_t expectedValue = y*bounds[1] + x;
      if(y < subspaceOffset[0] || y >= subspaceOffset[0] + subspaceSize[0]) expectedValue = fill_value;
      if(x < subspaceOffset[1] || x >= subspaceOffset[1] + subspaceSize[1]) expectedValue = fill_value;
      eassert(readBuffer[y][x] == expectedValue);
    }
  }

  //close down
  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  ret = esdm_finalize();
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n");

  return 0;
}
