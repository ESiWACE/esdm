/*
 * =====================================================================================
 *
 *       Filename:  test_vector.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/16/2016 06:07:51 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <netcdf_par.h>

#include "../../benchmark.h"


//void benchmark_init(benchmark_t* benchmark);
//void benchmark_setup(
//		benchmark_t* bm,
//		const procs_t procs,
//		const size_t ndims,
//		const size_t* dgeom,
//		const size_t* bgeom,
//		const size_t* cgeom,
//		const char* testfn,
//		const io_mode_t io_mode,
//		const int par_access,
//		const bool is_unlimited
//		);


int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	const size_t n = atoi(argv[1]);
	const size_t p = atoi(argv[2]);
	const procs_t procs = {n, p};
	const size_t t = atoi(argv[3]);
	const size_t x = atoi(argv[4]);
	const size_t y = atoi(argv[5]);
	const size_t z = atoi(argv[6]);
	const size_t ndims = 4;
	const size_t dgeom[4] = {t, x, y, z};
	const size_t bgeom[4] = {1, x / n, y / p, z};
	const size_t cgeom[4] = {1, x / n, y / p, z};
	const char* testfn = "./test.nc";
	const io_mode_t io_mode = NC_INDEPENDENT;
	const int par_access = NC_CHUNKED;
	const bool is_unlimited = 0;

	benchmark_t bm;
	benchmark_init(&bm);
	benchmark_setup(&bm, procs, ndims, dgeom, bgeom, cgeom, testfn, io_mode, par_access, is_unlimited);
	benchmark_run(&bm);
	benchmark_destroy(&bm);
	MPI_Finalize();
	return 0;
}
