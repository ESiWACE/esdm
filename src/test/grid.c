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

#include <stdio.h>
#include <esdm.h>
#include <esdm-datatypes-internal.h>
#include <esdm-debug.h>
#include <esdm-grid.h>

int main() {
  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  //The test data.
  const int64_t dimCount = 1;
  uint8_t data[3] = {0, 1, 2};
  esdm_simple_dspace_t dataspace = esdm_dataspace_1d(sizeof(data)/sizeof*data, SMD_DTYPE_UINT8);
  eassert(dataspace.ptr);

  //The file structure.
  esdm_container_t *container;
  ret = esdm_container_create("mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);
  esdm_dataset_t *dataset;
  ret = esdm_dataset_create(container, "mydataset", dataspace.ptr, &dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_commit(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_commit(container);
  eassert(ret == ESDM_SUCCESS);

  //Create some grids.
  //The grid structure is as follows:
  //  data position:   0       1       2
  //               |          top          |
  //               |---------------|       |
  //               |     proxy     |       |
  //               |---------------|       |
  //               |     split     |       |
  //               |       |-------|       |
  //               |       |  leaf |       |
  //
  //  * data point 0 is the first cell of the splitGrid
  //  * data point 1 is the only cell of the leafGrid
  //  * data point 2 is the second cell of the topGrid
  esdm_grid_t* topGrid, *proxyGrid, *splitGrid, *leafGrid;
  ret = esdm_grid_createSimple(dataset, dimCount, &(int64_t){3}, &topGrid);
  eassert(ret == ESDM_SUCCESS);
  eassert(topGrid);
  ret = esdm_grid_subdivideFixed(topGrid, 0, 2, true);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_grid_createSubgrid(topGrid, &(int64_t){0}, &proxyGrid);
  eassert(ret == ESDM_SUCCESS);
  eassert(proxyGrid);
  ret = esdm_grid_createSubgrid(proxyGrid, &(int64_t){0}, &splitGrid);
  eassert(ret == ESDM_SUCCESS);
  eassert(splitGrid);
  ret = esdm_grid_subdivideFlexible(splitGrid, 0, 2);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_grid_createSubgrid(splitGrid, &(int64_t){1}, &leafGrid);
  eassert(ret == ESDM_SUCCESS);
  eassert(leafGrid);

  //Write the data and check that the top grid is marked as complete only after all datapoints have been written.
  esdm_simple_dspace_t datapoint0 = esdm_dataspace_1do(0, 1, SMD_DTYPE_UINT8);
  esdm_simple_dspace_t datapoint1 = esdm_dataspace_1do(1, 1, SMD_DTYPE_UINT8);
  esdm_simple_dspace_t datapoint2 = esdm_dataspace_1do(2, 1, SMD_DTYPE_UINT8);

  ret = esdm_write_grid(proxyGrid, datapoint0.ptr, &data[0]);
  eassert(ret == ESDM_SUCCESS);
  int64_t gridCount;
  ret = esdm_dataset_grids(dataset, &gridCount, NULL);
  eassert(ret == ESDM_SUCCESS);
  eassert(gridCount == 0);

  ret = esdm_write_grid(topGrid, datapoint2.ptr, &data[2]);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_grids(dataset, &gridCount, NULL);
  eassert(ret == ESDM_SUCCESS);
  eassert(gridCount == 0);

  ret = esdm_write_grid(topGrid, datapoint1.ptr, &data[1]);
  eassert(ret == ESDM_SUCCESS);
  esdm_grid_t** grids;
  ret = esdm_dataset_grids(dataset, &gridCount, &grids);
  eassert(ret == ESDM_SUCCESS);
  eassert(gridCount == 1);
  eassert(grids[0] == topGrid);

  //Check that the grid information is actually stored to disk.
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_open(container, "mydataset", ESDM_MODE_FLAG_READ, &dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_grids(dataset, &gridCount, &grids);
  eassert(ret == ESDM_SUCCESS);
  eassert(gridCount == 1);
  topGrid = grids[0];

  //Iterate over the grid cells recursively and check the data.
  esdm_gridIterator_t* iterator;
  ret = esdm_gridIterator_create(topGrid, &iterator);
  while(true) {
    esdm_dataspace_t* curDataspace;
    ret = esdm_gridIterator_next(&iterator, 1, &curDataspace);
    eassert(ret == ESDM_SUCCESS);
    if(!curDataspace) break;
    eassert(curDataspace->dims == 1);
    eassert(curDataspace->size[0] == 1);
    eassert(curDataspace->offset[0] >= 0);
    eassert(curDataspace->offset[0] < 3);

    uint8_t fetchedData;
    ret = esdm_read_grid(topGrid, curDataspace, &fetchedData);
    eassert(ret == ESDM_SUCCESS);
    eassert((int64_t)fetchedData == curDataspace->offset[0]);

    ret = esdm_dataspace_destroy(curDataspace);
    eassert(ret == ESDM_SUCCESS);
  }
  eassert(!iterator);
  esdm_gridIterator_destroy(iterator);  //should be a noop

  //Cleanup.
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);

  printf("\nOK\n\n");
}
