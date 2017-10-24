/* This file is part of ESDM.
 *
 * This program is is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is is distributed in the hope that it will be useful,
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <esdm.h>


int verify_data(uint64_t* a, uint64_t* b) {
	int mismatches = 0;
	int idx;

	for(int x=0; x < 10; x++){
		for(int y=0; y < 20; y++){
			idx = y*10+x;
		
			if (a[idx] != b[idx]) {
				mismatches++;
				printf("idx=%04d, x=%04d, y=%04d should be %10ld but is %10ld\n", idx, x, y, a[idx], b[idx]);
			}
		}
	}


	return mismatches;
}


int main(){
	ESDM_status_t ret;
	int mismatches;
	// offset in the actual ND dimensions
	uint64_t offset[2] = {0, 0};
	// the size of the data to write
	uint64_t size[2] = {10, 20};

	ESDM_dataset_t dataset;


	// Prepare dummy data.
	uint64_t * buf_w = (uint64_t *) malloc(10*20*sizeof(uint64_t));
	uint64_t * buf_r = (uint64_t *) malloc(10*20*sizeof(uint64_t));


	for(int x=0; x < 10; x++){
		for(int y=0; y < 20; y++){
			buf_w[y*10+x] = (y+1)*10 + x + 1;
		}
	}


	// TODO: locate dataset metadata... We assume here the dataset is an uint64_t dataset

	// Write the data to the dataset
	ret = esdm_write(buf_w, dataset, 2, size, offset);


	// Read the data to the dataset
	ret = esdm_read(buf_r, dataset, 2, size, offset);


	// verify data and fail test if mistaches are found
	mismatches = verify_data(buf_w, buf_r);
	//assert(mismatches == 0);

	// clean up
	free(buf_w);
	free(buf_r);

	return 0;
}
