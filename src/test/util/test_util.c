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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "test_util.h"

#ifdef ESM

void start_timer(timer *t1) {
  *t1 = clock64();
}

double stop_timer(timer t1) {
  timer end;
  start_timer(&end);
  return (end - t1) / 1000.0 / 1000.0;
}

double timer_subtract(timer number, timer subtract) {
  return (number - subtract) / 1000.0 / 1000.0;
}

#else // POSIX COMPLAINT

void start_timer(timer *t1) {
  clock_gettime(CLOCK_MONOTONIC, t1);
}

static timer time_diff(struct timespec end, struct timespec start) {
  struct timespec diff;
  if (end.tv_nsec < start.tv_nsec) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return diff;
}

static double time_to_double(struct timespec t) {
  double d = (double)t.tv_nsec;
  d /= 1000000000.0;
  d += (double)t.tv_sec;
  return d;
}

double timer_subtract(timer number, timer subtract) {
  return time_to_double(time_diff(number, subtract));
}

double stop_timer(timer t1) {
  timer end;
  start_timer(&end);
  return time_to_double(time_diff(end, t1));
}


void etest_gen_buffer(int dims, int64_t bounds[], uint64_t ** out_buff){
  // prepare data
  int64_t cnt = 1;
  for(int d=0; d < dims; d++){
    cnt *= bounds[d];
  }

  uint64_t *buf_w = (uint64_t *)malloc(cnt * sizeof(uint64_t));
  assert(buf_w);
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

#endif
