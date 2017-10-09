/*
 * =====================================================================================
 *
 *       Filename:  bench.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/12/2016 06:46:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */

#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "benchmark.h"
#include "types.h"
#include "debug.h"
#include "timer.h"

#define FATAL_NC_ERR {if(err!=NC_NOERR) {printf("Error in file=%s at line=%d: %s Aborting ...\n", __FILE__, __LINE__, nc_strerror(err)); exit(-1);}}
#define NC_ERR \
	{ \
		if(err!=NC_NOERR) { \
			printf("Error in file=%s at line=%d: %s\n", __FILE__, __LINE__, nc_strerror(err)); \
			exit(-1); \
		} \
	}


/**
 * @brief  Error Handling
 *
 * @param status
 */
void handle_error(const int status) {
if (status != NC_NOERR) {
   fprintf(stderr, "%s\n", nc_strerror(status));
   exit(-1);
   }
}

/**
 * @brief
 *
 * @param nn
 * @param ppn
 *
 * @return
 */
void benchmark_init(benchmark_t* bm) {
	MPI_Comm_rank(MPI_COMM_WORLD, &bm->rank);
	MPI_Comm_size(MPI_COMM_WORLD, &bm->nranks);
	int len = 0;
	char processor[80];
	MPI_Get_processor_name(processor, &len);
	bm->processor = (char*)malloc(sizeof(*bm->processor) * len + 1);
	strcpy(bm->processor, processor);
	bm->testfn = malloc(0);
	bm->block = malloc(0);
  bm->duration.open = -1;
  bm->duration.io = -1;
  bm->duration.close = -1;
  bm->mssize = 0;
  bm->ms = malloc(0);
	bm->ndims = 0;
}



void benchmark_destroy(benchmark_t* bm) {
	free(bm->processor);
	bm->processor = NULL;
	free(bm->testfn);
	bm->testfn = NULL;
	free(bm->block);
	bm->block = NULL;
  free(bm->ms);
  bm->ms = NULL;
}



void benchmark_setup(
		benchmark_t* bm,
		const procs_t procs,
    const size_t ndims,
		const size_t* dgeom,
		const size_t* bgeom,
		const size_t* cgeom,
		const char* testfn,
		const io_mode_t io_mode,
		const int par_access,
		const bool is_unlimited,
		const int use_fill_value
		)
{
	assert(dgeom[DX] % procs.nn == 0);
	assert(dgeom[DY] % procs.ppn == 0);

	bm->use_fill_value = use_fill_value;
	bm->par_access = par_access;
	switch (bm->par_access) {
		case NC_COLLECTIVE:
			bm->is_unlimited = is_unlimited;
			break;
		case NC_INDEPENDENT:
			if (is_unlimited) {
				FATAL_ERR("Unlimited variables are not supported by independent I/O.\n");
			}
			bm->is_unlimited = false;
			break;
		default:
			FATAL_ERR("Bad par par access type.\n");
	}

  // processes = number_of_nodes * processes_per_node
	bm->procs = procs;

  // Data, block and chunk dimensions
	bm->ndims = ndims;
	for (size_t i = 0; i < bm->ndims; ++i) {
		bm->dgeom[i] = dgeom[i];
	}

	for (size_t i = 0; i < bm->ndims; ++i) {
		bm->bgeom[i] = bgeom[i];
	}

	if (NULL == cgeom) {
		bm->storage = NC_CONTIGUOUS;
	}
	else {
		for (size_t i = 0; i < bm->ndims; ++i) {
			bm->cgeom[i] = cgeom[i];
		}
		bm->storage = NC_CHUNKED;
	}

	// Testfile
	bm->io_mode = io_mode;
	bm->testfn = (char*)realloc(bm->testfn, sizeof(char*) * strlen(testfn) + 1);
	strcpy(bm->testfn, testfn);

  // Memory allocation for measurements
  bm->mssize = bm->dgeom[DT] / bm->bgeom[DT];
	bm->ms = (measurement_t*)realloc(bm->ms, sizeof(*bm->ms) * bm->mssize);

  // Memory allocation and initialization of block
	bm->block_size = bm->bgeom[DT] * bm->bgeom[DX] * bm->bgeom[DY] * bm->bgeom[DZ] * sizeof(DATATYPE);
	bm->block = (DATATYPE*) realloc(bm->block, bm->block_size);
	if(bm->block == NULL){
		DEBUG_MESSAGE("Could not allocate> %lu\n", bm->block_size);
		exit(1);
	}

	// INIT BLOCK
	typedef DATATYPE (*block_t)[bm->bgeom[DX]][bm->bgeom[DY]][bm->bgeom[DZ]];
	block_t block = (block_t) bm->block;
	for(size_t t = 0; t < bm->bgeom[DT]; ++t) {
		for(size_t x = 0; x < bm->bgeom[DX]; ++x) {
			for(size_t y = 0; y < bm->bgeom[DY]; ++y) {
				for(size_t z = 0; z < bm->bgeom[DZ]; ++z) {
					block[t][x][y][z] = (DATATYPE)(100 * t + bm->rank + x + y * 10 + 1);
				}
			}
		}
	}
}



int benchmark_run(benchmark_t* bm, DATATYPE* compare_block){
	int verify_results = 1;
	int err = 0;
	int cmode = 0;
	int ncid = 0;
	char dimname[80];
	int dimids[bm->ndims];
  int varid = 0;
	nc_x_vara_t nc_x_vara;

	switch (bm->io_mode) {
		case IO_MODE_WRITE:
			nc_x_vara = (nc_x_vara_t) nc_put_vara;
			break;
		case IO_MODE_READ:
			nc_x_vara = (nc_x_vara_t) nc_get_vara;
			break;
	}

	/* OPEN BENCHMARK */
	DEBUG_MESSAGE("OPEN_BENCHMARK\n");
	timespec_t start_open, stop_open;
	MPI_Barrier(MPI_COMM_WORLD);
	start_timer(&start_open);


	switch (bm->io_mode) {
		case IO_MODE_WRITE:
			cmode = NC_CLOBBER | NC_MPIIO | NC_NETCDF4;
			err = nc_create_par(bm->testfn, cmode, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid); FATAL_NC_ERR;

			if (! bm->use_fill_value){
				int old_fill_mode;
				err = nc_set_fill(ncid, NC_NOFILL, &old_fill_mode);
				FATAL_NC_ERR;
				err = nc_set_fill(ncid, NC_NOFILL, &old_fill_mode);
				if(old_fill_mode != NC_NOFILL){
					FATAL_ERR("ERROR setting no-fill mode\n");
				}
			}

			if (bm->is_unlimited) {
				err = nc_def_dim(ncid, "time", NC_UNLIMITED, &(dimids)[DT]); NC_ERR;
			}
			else {
				err = nc_def_dim(ncid, "time", bm->dgeom[DT], &dimids[DT]); NC_ERR;
			}

			for (size_t i = 1; i < bm->ndims; ++i) {
				sprintf(dimname, "dim_%zu", i);
				err = nc_def_dim(ncid, dimname, bm->dgeom[i], &dimids[i]); NC_ERR;
			}
			err = nc_def_var(ncid, "data", NC_DATATYPE, bm->ndims, dimids, &varid); NC_ERR;

			if (NC_CHUNKED == bm->storage) {
				err = nc_def_var_chunking(ncid, varid, NC_CHUNKED, bm->cgeom); FATAL_NC_ERR;
			}

			if (! bm->use_fill_value){
				err = nc_def_var_fill(ncid, varid, 1, & err); FATAL_NC_ERR;
			}
			err = nc_enddef(ncid); NC_ERR;
			break;
		case IO_MODE_READ:
			err = nc_open_par(bm->testfn, NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid); FATAL_NC_ERR;
			err = nc_inq_varid(ncid, "data", &varid); NC_ERR;
			break;
		default:
			perror("No io_mode (write/read) is selected");
			exit(-1);
	}

	err = nc_var_par_access(ncid, varid, bm->par_access); NC_ERR;

	MPI_Barrier(MPI_COMM_WORLD);
	start_timer(&stop_open);
	/* END: OPEN BENCHMARK */


	/* IO BENCHMARK */
	DEBUG_MESSAGE("IO_BENCHMARK[%d]\n", bm->rank);
	timespec_t start_io, stop_io;
	timespec_t start_io_slice, stop_io_slice;
	int i = 0;
	size_t start[] = {0, bm->bgeom[DX] * (bm->rank / bm->procs.ppn), bm->bgeom[DY] * (bm->rank % bm->procs.ppn), 0};

	MPI_Barrier(MPI_COMM_WORLD);
	start_timer(&start_io);
	for (size_t to = 0; to < bm->dgeom[DT]; to += bm->bgeom[DT]) {
		start[0] = to;

		DEBUG_MESSAGE("RUN offset %zu:%zu:%zu:%zu\n", start[DT], start[DX], start[DY], start[DZ]);

		start_timer(&start_io_slice);
		err = nc_x_vara(ncid, varid, start, bm->bgeom, (int*) bm->block); FATAL_NC_ERR;
		stop_timer(&stop_io_slice);

		if(compare_block != NULL){
			for (size_t i = 0; i < bm->block_size/sizeof(DATATYPE); i++) {
				if (abs((double) bm->block[i] - (double) compare_block[i]) > 0.001) {
					printf("ERROR %.3f (read) != %.3f (expected)\n", (double)  bm->block[i], (double) compare_block[i]);
					verify_results = 0;
				}
			}
		}

		bm->ms[i].time_offset = to;
		bm->ms[i].duration = time_to_double(stop_io_slice) - time_to_double(start_io_slice);
		++i;
	}
	MPI_Barrier(MPI_COMM_WORLD);
	stop_timer(&stop_io);
	/* END: IO BENCHMARK */



	/* CLOSE BENCHMARK */
	DEBUG_MESSAGE("CLOSE_BENCHMARK\n");
	timespec_t start_close, stop_close;
	MPI_Barrier(MPI_COMM_WORLD);
	start_timer(&start_close);
	nc_close(ncid);
	MPI_Barrier(MPI_COMM_WORLD);
	stop_timer(&stop_close);
	/* END: OPEN BENCHMARK */



	/* REPORT */
	bm->duration.open  = time_to_double(stop_open)  - time_to_double(start_open);
	bm->duration.io    = time_to_double(stop_io)    - time_to_double(start_io);
	bm->duration.close = time_to_double(stop_close) - time_to_double(start_close);
	/* END: REPORT */

	// Clean up
  return verify_results;
}
