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


#include <test/util/test_util.h>


#include <esdm.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  // Interaction with ESDM
  esdm_status status;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  status = esdm_init();
  eassert(status == ESDM_SUCCESS);

  status = esdm_container_open("mycontainer", &container);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_open(container, "mydataset", &dataset);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_close(dataset);
  eassert(status == ESDM_SUCCESS);

  esdm_dataset_t * dataset2 = NULL;
  status = esdm_dataset_open(container, "mydataset", &dataset2);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_open(container, "mydataset", &dataset);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_close(dataset);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_close(dataset2);
  eassert(status == ESDM_SUCCESS);

  status = esdm_container_close(container);
  eassert(status == ESDM_SUCCESS);


  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);

  printf("OK\n");

  return 0;
}
