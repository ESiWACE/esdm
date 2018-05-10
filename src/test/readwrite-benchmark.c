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
#include "util/test_util.h"

int verify_data(long size, uint64_t* a, uint64_t* b) {
	int mismatches = 0;
	int idx;

	for(int x=0; x < size; x++){
		for(int y=0; y < size; y++){
			idx = y*size+x;

			if (a[idx] != b[idx]) {
				mismatches++;
			}
		}
	}
	return mismatches;
}


int main(int argc, char const* argv[])
{
	if(argc != 2){
		printf("Syntax: %s SIZE", argv[0]);
		printf("\t SIZE specifies one dimension of a 2D field\n");
		exit(1);
	}
	const long size = atol(argv[1]);
	printf("Running with a 2D slice of %ld*%ld\n", size, size);
	const long volume = size*size*sizeof(uint64_t);

	// prepare data
	uint64_t * buf_w = (uint64_t *) malloc(volume);
	uint64_t * buf_r = (uint64_t *) malloc(volume);

	for(int x=0; x < size; x++){
		for(int y=0; y < size; y++){
			buf_w[y*size+x] = y*size + x + 1;
		}
	}


	// Interaction with ESDM
	esdm_status_t ret;
	esdm_container_t *container = NULL;
	esdm_dataset_t *dataset = NULL;


	esdm_init();

	// define dataspace
	int64_t bounds[] = {size, size};
	esdm_dataspace_t *dataspace = esdm_dataspace_create(2, bounds, esdm_uint64_t);

	container = esdm_container_create("mycontainer");
	dataset = esdm_dataset_create(container, "mydataset", dataspace);


	esdm_container_commit(container);
	esdm_dataset_commit(dataset);


	// define subspace
	int64_t dim[] = {size,size};
	int64_t offset[] = {0,0};
	esdm_dataspace_t *subspace = esdm_dataspace_subspace(dataspace, 2, dim, offset);

	timer t;
	start_timer(&t);

	// Write the data to the dataset
	ret = esdm_write(dataset, buf_w, subspace);
	const double write_time = stop_timer(t);

	start_timer(&t);
	// Read the data to the dataset
	ret = esdm_read(dataset, buf_r, subspace);
	const double read_time = stop_timer(t);

	// verify data and fail test if mismatches are found
	int mismatches = verify_data(size, buf_w, buf_r);
	printf("Mismatches: %d\n", mismatches);
	if ( mismatches > 0 ) {
		printf("FAILED\n");
	} else {
		printf("OK\n");
	}
	assert(mismatches == 0);

	printf("Runtime read,write: %.3f,%.3f\n", read_time, write_time);
	printf("Performance read,write: %.3f MiB/s,%.3f MiB/s\n", volume/read_time/1024/1024, volume / write_time / 1024.0 / 1024);


	// clean up
	free(buf_w);
	free(buf_r);

	return 0;
}
