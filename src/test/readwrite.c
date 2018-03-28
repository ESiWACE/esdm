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
				//printf("idx=%04d, x=%04d, y=%04d should be %10ld but is %10ld\n", idx, x, y, a[idx], b[idx]);
			}
		}
	}


	return mismatches;
}


int main(int argc, char const* argv[])
{

	// prepare data
	uint64_t * buf_w = (uint64_t *) malloc(10*20*sizeof(uint64_t));
	uint64_t * buf_r = (uint64_t *) malloc(10*20*sizeof(uint64_t));

	for(int x=0; x < 10; x++){
		for(int y=0; y < 20; y++){
			buf_w[y*10+x] = (y)*10 + x + 1;
		}
	}


	// Interaction with ESDM
	esdm_status_t ret;
	esdm_container_t *container = NULL;
	esdm_dataset_t *dataset = NULL;


	esdm_init();


	//ret = esdm_create("mytextfile", ESDM_CREATE, &container, &dataset);
	//esdm_open("mycontainer/mydataset", ESDM_CREATE);
	
	// POSIX pwrite/pread interfaces for comparison
	//ssize_t pread(int fd, void *buf, size_t count, off_t offset);
    //ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);



	// define dataspace
	int64_t bounds[] = {10, 20};
	esdm_dataspace_t *dataspace = esdm_dataspace_create(2, bounds, esdm_uint64_t);

	container = esdm_container_create("mycontainer");
	dataset = esdm_dataset_create(container, "mydataset", dataspace);

	
	esdm_container_commit(container);
	esdm_dataset_commit(dataset);


	// define subspace
	int64_t size[] = {10,20};
	int64_t offset[] = {0,0};
	esdm_dataspace_t *subspace = esdm_dataspace_subspace(dataspace, 2, size, offset);


	// Write the data to the dataset
	ret = esdm_write(dataset, buf_w, subspace);

	// Read the data to the dataset
	ret = esdm_read(dataset, buf_r, subspace);




	// TODO: write subset
	// TODO: read subset -> subspace reconstruction



	// verify data and fail test if mismatches are found
	int mismatches = verify_data(buf_w, buf_r);
	printf("Mismatches: %d\n", mismatches);
	if ( mismatches > 0 ) {
		printf("FAILED\n");
	} else {
		printf("OK\n");
	}
	//assert(mismatches == 0);


	// clean up
	free(buf_w);
	free(buf_r);

	return 0;
}
