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

void checkRanges() {
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

void checkHypercubes() {
  int64_t
    offsetA[3] = { -4, -3, 0 },
    sizeA[3] = { 8, 6, 4 },
    offsetB[3] = { -1, -1, -100 },
    sizeB[3] = { 2, 2, 200 },
    offsetC[3] = { 0, 0, 0 },
    sizeC[3] = { 10, 10, 10 },
    offsetD[3] = { 0, 0, 0 },
    sizeD[3] = { 0, 0, 0 };

  eassert_crash(esdmI_hypercube_make(3, NULL, sizeA));
  eassert_crash(esdmI_hypercube_make(3, offsetA, NULL));
  esdmI_hypercube_t* cubeA = esdmI_hypercube_make(3, offsetA, sizeA);
  eassert(cubeA);
  esdmI_hypercube_t* cubeB = esdmI_hypercube_make(3, offsetB, sizeB);
  eassert(cubeB);
  esdmI_hypercube_t* cubeC = esdmI_hypercube_make(3, offsetC, sizeC);
  eassert(cubeC);
  esdmI_hypercube_t* cubeD = esdmI_hypercube_make(3, offsetD, sizeD);
  eassert(cubeD);

  eassert_crash(esdmI_hypercube_makeCopy(NULL));
  esdmI_hypercube_t* cubeE = esdmI_hypercube_makeCopy(cubeD);
  eassert(cubeE);

  eassert_crash(esdmI_hypercube_makeIntersection(NULL, cubeA));
  eassert_crash(esdmI_hypercube_makeIntersection(cubeA, NULL));
  esdmI_hypercube_t* intersectionAA = esdmI_hypercube_makeIntersection(cubeA, cubeA);
  eassert( intersectionAA);
  eassert( esdmI_hypercube_doesIntersect(cubeA, cubeA));
  esdmI_hypercube_t* intersectionAB = esdmI_hypercube_makeIntersection(cubeA, cubeB);
  eassert( intersectionAB);
  eassert( esdmI_hypercube_doesIntersect(cubeA, cubeB));
  esdmI_hypercube_t* intersectionAC = esdmI_hypercube_makeIntersection(cubeA, cubeC);
  eassert( intersectionAC);
  eassert( esdmI_hypercube_doesIntersect(cubeA, cubeC));
  esdmI_hypercube_t* intersectionAD = esdmI_hypercube_makeIntersection(cubeA, cubeD);
  eassert(!intersectionAD);
  eassert(!esdmI_hypercube_doesIntersect(cubeA, cubeD));
  esdmI_hypercube_t* intersectionAE = esdmI_hypercube_makeIntersection(cubeA, cubeE);
  eassert(!intersectionAE);
  eassert(!esdmI_hypercube_doesIntersect(cubeA, cubeE));
  esdmI_hypercube_t* intersectionBA = esdmI_hypercube_makeIntersection(cubeB, cubeA);
  eassert( intersectionBA);
  eassert( esdmI_hypercube_doesIntersect(cubeB, cubeA));
  esdmI_hypercube_t* intersectionBB = esdmI_hypercube_makeIntersection(cubeB, cubeB);
  eassert( intersectionBB);
  eassert( esdmI_hypercube_doesIntersect(cubeB, cubeB));
  esdmI_hypercube_t* intersectionBC = esdmI_hypercube_makeIntersection(cubeB, cubeC);
  eassert( intersectionBC);
  eassert( esdmI_hypercube_doesIntersect(cubeB, cubeC));
  esdmI_hypercube_t* intersectionBD = esdmI_hypercube_makeIntersection(cubeB, cubeD);
  eassert(!intersectionBD);
  eassert(!esdmI_hypercube_doesIntersect(cubeB, cubeD));
  esdmI_hypercube_t* intersectionBE = esdmI_hypercube_makeIntersection(cubeB, cubeE);
  eassert(!intersectionBE);
  eassert(!esdmI_hypercube_doesIntersect(cubeB, cubeE));
  esdmI_hypercube_t* intersectionCA = esdmI_hypercube_makeIntersection(cubeC, cubeA);
  eassert( intersectionCA);
  eassert( esdmI_hypercube_doesIntersect(cubeC, cubeA));
  esdmI_hypercube_t* intersectionCB = esdmI_hypercube_makeIntersection(cubeC, cubeB);
  eassert( intersectionCB);
  eassert( esdmI_hypercube_doesIntersect(cubeC, cubeB));
  esdmI_hypercube_t* intersectionCC = esdmI_hypercube_makeIntersection(cubeC, cubeC);
  eassert( intersectionCC);
  eassert( esdmI_hypercube_doesIntersect(cubeC, cubeC));
  esdmI_hypercube_t* intersectionCD = esdmI_hypercube_makeIntersection(cubeC, cubeD);
  eassert(!intersectionCD);
  eassert(!esdmI_hypercube_doesIntersect(cubeC, cubeD));
  esdmI_hypercube_t* intersectionCE = esdmI_hypercube_makeIntersection(cubeC, cubeE);
  eassert(!intersectionCE);
  eassert(!esdmI_hypercube_doesIntersect(cubeC, cubeE));
  esdmI_hypercube_t* intersectionDA = esdmI_hypercube_makeIntersection(cubeD, cubeA);
  eassert(!intersectionDA);
  eassert(!esdmI_hypercube_doesIntersect(cubeD, cubeA));
  esdmI_hypercube_t* intersectionDB = esdmI_hypercube_makeIntersection(cubeD, cubeB);
  eassert(!intersectionDB);
  eassert(!esdmI_hypercube_doesIntersect(cubeD, cubeB));
  esdmI_hypercube_t* intersectionDC = esdmI_hypercube_makeIntersection(cubeD, cubeC);
  eassert(!intersectionDC);
  eassert(!esdmI_hypercube_doesIntersect(cubeD, cubeC));
  esdmI_hypercube_t* intersectionDD = esdmI_hypercube_makeIntersection(cubeD, cubeD);
  eassert(!intersectionDD);
  eassert(!esdmI_hypercube_doesIntersect(cubeD, cubeD));
  esdmI_hypercube_t* intersectionDE = esdmI_hypercube_makeIntersection(cubeD, cubeE);
  eassert(!intersectionDE);
  eassert(!esdmI_hypercube_doesIntersect(cubeD, cubeE));
  esdmI_hypercube_t* intersectionEA = esdmI_hypercube_makeIntersection(cubeE, cubeA);
  eassert(!intersectionEA);
  eassert(!esdmI_hypercube_doesIntersect(cubeE, cubeA));
  esdmI_hypercube_t* intersectionEB = esdmI_hypercube_makeIntersection(cubeE, cubeB);
  eassert(!intersectionEB);
  eassert(!esdmI_hypercube_doesIntersect(cubeE, cubeB));
  esdmI_hypercube_t* intersectionEC = esdmI_hypercube_makeIntersection(cubeE, cubeC);
  eassert(!intersectionEC);
  eassert(!esdmI_hypercube_doesIntersect(cubeE, cubeC));
  esdmI_hypercube_t* intersectionED = esdmI_hypercube_makeIntersection(cubeE, cubeD);
  eassert(!intersectionED);
  eassert(!esdmI_hypercube_doesIntersect(cubeE, cubeD));
  esdmI_hypercube_t* intersectionEE = esdmI_hypercube_makeIntersection(cubeE, cubeE);
  eassert(!intersectionEE);
  eassert(!esdmI_hypercube_doesIntersect(cubeE, cubeE));
}

int main() {
  checkRanges();
  checkHypercubes();
}
