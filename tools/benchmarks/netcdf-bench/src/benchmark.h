/*
 * =====================================================================================
 *
 *       Filename:  bench.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/12/2016 06:46:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */

#ifndef  bench_INC
#define  bench_INC

#define xstr(s) str(s)
#define str(s) #s
#define str_append(x,y) x#y

#include <stdbool.h>

#include "types.h"
#include "constants.h"

typedef int64_t int64;
typedef unsigned char byte;

typedef int (*nc_x_vara_t)(int, int, const size_t*, const size_t*, int*);

typedef struct benchmark_t {
	char* processor;
	char* testfn;
	DATATYPE * block;
	measurement_t* ms;
	int nranks;
	int rank;
	size_t ndims;
	size_t dgeom[NDIMS];
	size_t bgeom[NDIMS];
	size_t cgeom[NDIMS];
	procs_t procs;
	io_mode_t io_mode;
	size_t block_size;
	duration_t duration;
	size_t mssize;
	int par_access;
	int storage;
	bool is_unlimited;
	bool use_fill_value;
} benchmark_t;

void benchmark_init(benchmark_t* benchmark);
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
		);
int benchmark_run(benchmark_t* benchmark, DATATYPE* compare_block);
void benchmark_destroy(benchmark_t* benchmark);

void handle_error(const int status);
//int run(const procs_t proc_dims, const size_t* data_dims, const size_t* cgeom, const char* testfn, const io_mode_t io_mode, const int par_access, const bool is_unlimited);

#endif   /* ----- #ifndef bench_INC  ----- */
