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
#include <mpi.h>

#include <esdm.h>
#include "util/test_util.h"

int tasks_per_node() {
    MPI_Comm shared_comm;
    int count;

    MPI_Comm_split_type (MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shared_comm);
    MPI_Comm_size (shared_comm, &count);
    MPI_Comm_free (&shared_comm);

    return count;
}


int main(int argc, char* argv[])
{
  int provided;
	MPI_Init_thread(& argc, & argv, MPI_THREAD_FUNNELED, & provided);

	int mpi_size;
	int mpi_rank;

	MPI_Comm_rank(MPI_COMM_WORLD, & mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, & mpi_size);

	int64_t _size;
	if (argc < 2)
		_size = 1024;
	else if (argc != 3){
		printf("Syntax: %s [SIZE] [CONFIG]", argv[0]);
		printf("\t SIZE specifies one dimension of a 2D field\n");
		exit(1);
	} else
		_size = atoll(argv[1]);
	const int64_t size = _size;

	if(mpi_rank == 0)
		printf("Running with a 2D slice of %ld*%ld\n", size, size);

	if(size / mpi_size == 0){
		printf("Error, size < number of ranks!\n");
		exit(1);
	}

	int64_t dim[] = {size / mpi_size + (mpi_rank < (size % mpi_size) ? 1 : 0), size};
	int64_t offset[] = {size / mpi_size * mpi_rank + (mpi_rank < (size % mpi_size) ? mpi_rank : size % mpi_size), 0};
	//printf("%d %d - %d-%d \n", dim[1], dim[0], offset[1], offset[0]);

	const long volume 		= dim[0]*dim[1]*sizeof(uint64_t);
	const long volume_all = size*size*sizeof(uint64_t);

	// prepare data
	uint64_t * buf_w = (uint64_t *) malloc(volume);
	uint64_t * buf_r = (uint64_t *) malloc(volume);
	assert(buf_w != NULL);
	assert(buf_r != NULL);

	int x, y;
	for(y = offset[0]; y < dim[0]; y++){
		for(x = offset[1]; x < dim[1]; x++){
			buf_w[(y-offset[0])*size+x] = y*size + x + 1;
		}
	}

	// Interaction with ESDM
	esdm_status_t ret;
	esdm_container_t *container = NULL;
	esdm_dataset_t *dataset = NULL;

	int pPerNode = tasks_per_node();
  if (mpi_rank == 0)
	   printf("Running with %d processes per Node\n", pPerNode);
	esdm_set_procs_per_node(pPerNode);

	// TODO provide a support MPI library function to do this
	char * config = NULL;
	if (mpi_rank == 0){
		int len;
		read_file(argv[2], & config);
		len = strlen(config) + 1;
		MPI_Bcast(& len, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	}else{
		int len;
		MPI_Bcast(& len, 1, MPI_INT, 0, MPI_COMM_WORLD);
		config = (char*) malloc(len);
		MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	}
	esdm_load_config_str(config);
	// END TODO

	ret = esdm_init();

	// define dataspace
	int64_t bounds[] = {size, size};
	esdm_dataspace_t *dataspace = esdm_dataspace_create(2, bounds, esdm_uint64_t);

	container = esdm_container_create("mycontainer");
	dataset = esdm_dataset_create(container, "mydataset", dataspace);


	esdm_container_commit(container);
	esdm_dataset_commit(dataset);

	// define subspace
	esdm_dataspace_t *subspace = esdm_dataspace_subspace(dataspace, 2, dim, offset);

	timer t;
	MPI_Barrier(MPI_COMM_WORLD);
	start_timer(&t);

	// Write the data to the dataset
	ret = esdm_write(dataset, buf_w, subspace);
	assert( ret == ESDM_SUCCESS );
	MPI_Barrier(MPI_COMM_WORLD);
	const double write_time = stop_timer(t);

	start_timer(&t);
	// Read the data to the dataset
	ret = esdm_read(dataset, buf_r, subspace);
	assert( ret == ESDM_SUCCESS );

	MPI_Barrier(MPI_COMM_WORLD);
	const double read_time = stop_timer(t);

	// verify data and fail test if mismatches are found
	int mismatches = 0;
	int idx;

	for(y = offset[0]; y < dim[0]; y++){
		for(x = offset[1]; x < dim[1]; x++){
			idx = (y-offset[0])*size + x;

			if (buf_r[idx] != buf_w[idx]) {
				mismatches++;
			}
		}
	}

	double total_time_w;
	double total_time_r;
	MPI_Reduce(& write_time, &total_time_w, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(& read_time, &total_time_r, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	int mismatches_sum;
	MPI_Reduce(& mismatches, &mismatches_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (mpi_rank == 0){
		if ( mismatches_sum > 0 ) {
			printf("FAILED\n");
			printf("Mismatches: %d\n", mismatches_sum);
		} else {
			printf("OK\n");
		}
		assert(mismatches_sum == 0);

		printf("Runtime read,write: %.3f,%.3f\n", total_time_r, total_time_w);
		printf("Performance read,write: %.3f MiB/s,%.3f MiB/s size:%.0f MiB\n", volume_all/total_time_r/1024/1024, volume_all/total_time_w/1024.0/1024, volume_all/1024.0/1024);
	}


	// clean up
	free(buf_w);
	free(buf_r);

	MPI_Finalize();

	return 0;
}
