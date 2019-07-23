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
 * This test checks the sanity of the esdmI_hypercube_t implementation.
 */

#include <esdm-internal.h>
#include <test/util/test_util.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define checkRange(a, min, max) do {\
  esdmI_range_t temp_a = (a);\
  esdmI_range_t temp_b = (esdmI_range_t){ .start = min, .end = max };\
  eassert(temp_a.start == temp_b.start);\
  eassert(temp_a.end == temp_b.end);\
} while(0)

int main() {
  esdmI_range_t
    a = { .start = -3, .end = 3 },
    b = { .start = -5, .end = -5 },
    c = { .start = -5, .end = -4 },
    d = { .start = -5, .end = -3 },
    e = { .start = -5, .end = 3 },
    f = { .start = -5, .end = 4 },
    g = { .start = -3, .end = -3 },
    h = { .start = -3, .end = -2 },
    i = { .start = -3, .end = 3 },
    j = { .start = -3, .end = 4 },
    k = { .start = 3, .end = 3 },
    l = { .start = 3, .end = 4 },
    m = { .start = 3, .end = 2 };

  checkRange(esdmI_range_intersection(a, b), -3, -5);
  checkRange(esdmI_range_intersection(a, c), -3, -4);
  checkRange(esdmI_range_intersection(a, d), -3, -3);
  checkRange(esdmI_range_intersection(a, e), -3, 3);
  checkRange(esdmI_range_intersection(a, f), -3, 3);
  checkRange(esdmI_range_intersection(a, g), -3, -3);
  checkRange(esdmI_range_intersection(a, h), -3, -2);
  checkRange(esdmI_range_intersection(a, i), -3, 3);
  checkRange(esdmI_range_intersection(a, j), -3, 3);
  checkRange(esdmI_range_intersection(a, k), 3, 3);
  checkRange(esdmI_range_intersection(a, l), 3, 3);
  checkRange(esdmI_range_intersection(a, m), 3, 2);

  eassert(!esdmI_range_isEmpty(a));
  eassert( esdmI_range_isEmpty(b));
  eassert(!esdmI_range_isEmpty(c));
  eassert(!esdmI_range_isEmpty(d));
  eassert(!esdmI_range_isEmpty(e));
  eassert(!esdmI_range_isEmpty(f));
  eassert( esdmI_range_isEmpty(g));
  eassert(!esdmI_range_isEmpty(h));
  eassert(!esdmI_range_isEmpty(i));
  eassert(!esdmI_range_isEmpty(j));
  eassert( esdmI_range_isEmpty(k));
  eassert(!esdmI_range_isEmpty(l));
  eassert( esdmI_range_isEmpty(m));

  eassert(esdmI_range_size(a) == 6);
  eassert(esdmI_range_size(b) == 0);
  eassert(esdmI_range_size(c) == 1);
  eassert(esdmI_range_size(d) == 2);
  eassert(esdmI_range_size(e) == 8);
  eassert(esdmI_range_size(f) == 9);
  eassert(esdmI_range_size(g) == 0);
  eassert(esdmI_range_size(h) == 1);
  eassert(esdmI_range_size(i) == 6);
  eassert(esdmI_range_size(j) == 7);
  eassert(esdmI_range_size(k) == 0);
  eassert(esdmI_range_size(l) == 1);
  eassert(esdmI_range_size(m) == 0);
}
