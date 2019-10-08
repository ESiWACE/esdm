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
 * This test is a proof of concept for arbitrary hypercube memory mappings.
 */

#include <esdm.h>
#include <test/util/test_util.h>

#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  esdm_dataspace_t *logicalSpace;
  esdm_status result = esdm_dataspace_create(3, (int64_t[3]){8, 3, 14}, SMD_DTYPE_INT64, &logicalSpace);
  assert(result == ESDM_SUCCESS);

  esdm_dataspace_t *sourceSubspace;
  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){2, 3, 5}, (int64_t[3]){6, 0, 9}, &sourceSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(sourceSubspace, (int64_t[3]){-3, 1, 6});
  assert(result == ESDM_SUCCESS);
  int64_t sourceData[30] = {
    0x709, 0x719, 0x729,
    0x609, 0x619, 0x629,

    0x70a, 0x71a, 0x72a,
    0x60a, 0x61a, 0x62a,

    0x70b, 0x71b, 0x72b,
    0x60b, 0x61b, 0x62b,

    0x70c, 0x71c, 0x72c,
    0x60c, 0x61c, 0x62c,

    0x70d, 0x71d, 0x72d,
    0x60d, 0x61d, 0x62d
  }, *firstSourceByte = &sourceData[3];

  esdm_dataspace_t *destSubspace;
  result = esdm_dataspace_subspace(logicalSpace, 3, (int64_t[3]){3, 3, 3}, (int64_t[3]){5, 0, 10}, &destSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_set_stride(destSubspace, (int64_t[3]){-3, 1, -9});
  assert(result == ESDM_SUCCESS);
  int64_t destData[27] = {0}, expectedData[27] = {
    0x70c, 0x71c, 0x72c,
    0x60c, 0x61c, 0x62c,
    0x000, 0x000, 0x000,

    0x70b, 0x71b, 0x72b,
    0x60b, 0x61b, 0x62b,
    0x000, 0x000, 0x000,

    0x70a, 0x71a, 0x72a,
    0x60a, 0x61a, 0x62a,
    0x000, 0x000, 0x000,
  }, *firstDestByte = &destData[6 + 2*9];

  result = esdm_dataspace_copy_data(sourceSubspace, firstSourceByte, destSubspace, firstDestByte);
  assert(result == ESDM_SUCCESS);

  for(int64_t i = 0; i < 27; i++) eassert(destData[i] == expectedData[i]);

  result = esdm_dataspace_destroy(logicalSpace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_destroy(sourceSubspace);
  assert(result == ESDM_SUCCESS);
  result = esdm_dataspace_destroy(destSubspace);
  assert(result == ESDM_SUCCESS);

  printf("\nOK\n");
}
