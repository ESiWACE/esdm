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
#include <string.h>

#include <mpi.h>

#include <esdm.h>
#include "util/test_util.h"

int esdm_mpi_get_tasks_per_node() {
	MPI_Comm shared_comm;
	int count = 1;

	MPI_Comm_split_type (MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shared_comm);
	MPI_Comm_size (shared_comm, &count);
	MPI_Comm_free (&shared_comm);

	return count;
}

void esdm_mpi_distribute_config_file(char * config_filename){
	int mpi_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, & mpi_rank);

	char * config = NULL;
	if (mpi_rank == 0){
		int len;
		read_file(config_filename, & config);
		len = strlen(config) + 1;
		MPI_Bcast(& len, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	} else {
		int len;
		MPI_Bcast(& len, 1, MPI_INT, 0, MPI_COMM_WORLD);
		config = (char*) malloc(len);
		MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	}
	esdm_load_config_str(config);
}

void esdm_mpi_init(){
	int mpi_size;
	MPI_Comm_size(MPI_COMM_WORLD, & mpi_size);

	int pPerNode = esdm_mpi_get_tasks_per_node();
	esdm_set_procs_per_node(pPerNode);
	esdm_set_total_procs(mpi_size);
}


int main(int argc, char* argv[])
{
	int provided;
	MPI_Init_thread(& argc, & argv, MPI_THREAD_FUNNELED, & provided);

	int mpi_size;
	int mpi_rank;
	int run_read = 0;
	int run_write = 0;
	long timesteps = 0;
	int cycleBlock = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, & mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, & mpi_size);

	int64_t _size;
	char *config_file;
	char * default_args[] = {argv[0], "1024", "_esdm.conf", "B", "10"};
	if (argc == 1){
		argc = 5;
		argv = default_args;
	}
	if (argc != 5) {
		printf("Syntax: %s [SIZE] [CONFIG] [B|R|W][C] [TIMESTEPS]", argv[0]);
		printf("\t SIZE specifies one dimension of a 2D field\n");
		exit(1);
	}

	_size = atol(argv[1]);
	config_file = argv[2];
	switch(argv[3][0]){
		case('R'):{
			run_read = 1;
			break;
		}
		case('W'):{
			run_write = 1;
			break;
		}
		case('B'):{
			run_read = 1;
			run_write = 1;
			break;
		}
		default : {
			printf("Unknown setting for argument: %s expected [R|W|B]\n", argv[3]);
			exit(1);
		}
	}
	cycleBlock = argv[3][1] == 'C';
	timesteps = atol(argv[4]);

	const int64_t size = _size;

	if (mpi_rank == 0)
		printf("Running with %ld timesteps and 2D slice of %ld*%ld (cycle: %d)\n", timesteps, size, size, cycleBlock);

	if (size / mpi_size == 0){
		printf("Error, size < number of ranks!\n");
		exit(1);
	}

	int pPerNode = esdm_mpi_get_tasks_per_node();
	int tmp_rank = (mpi_rank + (cycleBlock*pPerNode)) % mpi_size;
	int64_t dim[] = {1, size / mpi_size + (tmp_rank < (size % mpi_size) ? 1 : 0), size};
	int64_t offset[] = {0, size / mpi_size * tmp_rank + (tmp_rank < (size % mpi_size) ? tmp_rank : size % mpi_size), 0};

	const long volume = dim[1] * dim[2] * sizeof(uint64_t);
	const long volume_all = timesteps * size * size * sizeof(uint64_t);

	if (!volume_all) {
		printf("Error: no data!\n");
		exit(1);
	}

	// prepare data
	uint64_t * buf_w = (uint64_t *) malloc(volume);
	assert(buf_w != NULL);
	long x, y;
	for(y = offset[1]; y < dim[1]; y++){
		for(x = offset[2]; x < dim[2]; x++){
			long idx = (y - offset[1]) * size + x;
			buf_w[idx] = y * size + x + 1 + mpi_rank;
		}
	}

	// Interaction with ESDM
	esdm_status ret;
	esdm_container *container = NULL;
	esdm_dataset_t *dataset = NULL;

	esdm_mpi_init();
	esdm_mpi_distribute_config_file(config_file);

	ret = esdm_init();
	assert( ret == ESDM_SUCCESS );

	// define dataspace
	int64_t bounds[] = {timesteps, size, size};
	esdm_dataspace_t *dataspace = esdm_dataspace_create(3, bounds, SMD_DTYPE_UINT64);

	container = esdm_container_create("mycontainer");
	dataset = esdm_dataset_create(container, "mydataset", dataspace);

	esdm_container_commit(container);
	esdm_dataset_commit(dataset);

	// define subspace

	timer t;
	double time;

	if(run_write){
		MPI_Barrier(MPI_COMM_WORLD);
		start_timer(&t);
		// Write the data to the dataset
		for(int t=0; t < timesteps; t++){
			offset[0] = t;
			esdm_dataspace_t *subspace = esdm_dataspace_subspace(dataspace, 3, dim, offset);

			ret = esdm_write(dataset, buf_w, subspace);
			assert( ret == ESDM_SUCCESS );
		}

		MPI_Barrier(MPI_COMM_WORLD);
		time = stop_timer(t);
		double total_time;
		MPI_Reduce((void *)& time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
		if (mpi_rank == 0){
			printf("Write: %.3fs %.3f MiB/s size:%.0f MiB\n", total_time, volume_all/total_time/1024.0/1024, volume_all/1024.0/1024);
		}
	}
	int mismatches = 0;

	if(run_read){
		MPI_Barrier(MPI_COMM_WORLD);
		start_timer(&t);
		// Read the data to the dataset
		for(int t=0; t < timesteps; t++){
			offset[0] = t;
			uint64_t * buf_r = (uint64_t *) malloc(volume);
			assert(buf_r != NULL);

			esdm_dataspace_t *subspace = esdm_dataspace_subspace(dataspace, 3, dim, offset);
			ret = esdm_read(dataset, buf_r, subspace);
			assert( ret == ESDM_SUCCESS );
			// verify data and fail test if mismatches are found
			long idx;
			for(y = offset[1]; y < dim[1]; y++){
				for(x = offset[2]; x < dim[2]; x++){
					idx = (y - offset[1]) * size + x;

					if (buf_r[idx] != buf_w[idx]) {
						mismatches++;
					}
				}
			}
			free(buf_r);
		}
		MPI_Barrier(MPI_COMM_WORLD);
 		time = stop_timer(t);

		int mismatches_sum;
		MPI_Reduce(& mismatches, &mismatches_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		double total_time;
		MPI_Reduce((void *)& time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
		if (mpi_rank == 0){
			if ( mismatches_sum > 0 ) {
				printf("FAILED\n");
				printf("Mismatches: %d\n", mismatches_sum);
			} else {
				printf("OK\n");
			}
			printf("Read: %.3fs %.3f MiB/s size:%.0f MiB\n", total_time, volume_all/total_time/1024.0/1024, volume_all/1024.0/1024);
		}
	}

	// clean up
	free(buf_w);

	MPI_Finalize();

	return 0;
}
