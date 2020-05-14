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

#include <esdm-debug.h>
#include <esdm-internal.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void etest_gen_buffer(int dims, int64_t bounds[], uint64_t ** out_buff){
  // prepare data
  int64_t cnt = 1;
  for(int d=0; d < dims; d++){
    cnt *= bounds[d];
  }

  uint64_t *buf_w = ea_checked_malloc(cnt * sizeof(uint64_t));
  eassert(buf_w);
  *out_buff = buf_w;

  for(int64_t i=0; i < cnt; i++){
    buf_w[i] = i + 1;
  }
}

void etest_memset_buffer(int dims, int64_t bounds[], uint64_t * buff){
  // prepare data
  int64_t cnt = 1;
  for(int d=0; d < dims; d++){
    cnt *= bounds[d];
  }

  memset(buff, 0, cnt * sizeof(uint64_t));
}


int etest_verify_buffer(int dims, int64_t bounds[], uint64_t * buff){
  // prepare data
  int64_t cnt = 1;
  for(int d=0; d < dims; d++){
    cnt *= bounds[d];
  }
  int64_t errors = 0;
  for(int64_t i=0; i < cnt; i++){
    if(buff[i] != i + 1){
      errors++;
    }
  }
  if(errors != 0){
    printf("Mismatches: %ld\n", errors);
    if (errors > 0) {
      printf("FAILED\n");
    } else {
      printf("OK\n");
    }
    return 1;
  }
  return 0;
}
