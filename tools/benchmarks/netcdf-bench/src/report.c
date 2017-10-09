/*
 * =====================================================================================
 *
 *       Filename:  report.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/18/2016 10:06:57 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */

#include "report.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>




/**
 * @brief 
 *
 * @param bm
 * @param tag
 */
static void benchmark_send(benchmark_t* bm, const int tag) {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int t;
	if (0 != rank) {
		t = tag;
		MPI_Send(bm, sizeof(benchmark_t), MPI_BYTE, 0, t, MPI_COMM_WORLD);
		++t;
		MPI_Send(bm->processor, strlen(bm->processor) + 1, MPI_CHAR, 0, t, MPI_COMM_WORLD);
		++t;
		MPI_Send(bm->testfn, strlen(bm->testfn) + 1, MPI_CHAR, 0, t,  MPI_COMM_WORLD);
		++t;
		MPI_Send(bm->ms, sizeof(bm->ms[0]) * bm->mssize, MPI_BYTE, 0, t, MPI_COMM_WORLD);
	}
}



/**
 * @brief 
 *
 * @param bm
 * @param tag
 */
static void benchmark_receive(benchmark_t** bm, const int tag) {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (0 == rank) {
		int nranks;
		MPI_Comm_size(MPI_COMM_WORLD, &nranks);
		MPI_Status status;
		int t;
		int count;
		for (int i = 1; i < nranks; ++i) {
			t = tag;
			MPI_Recv(bm[i], sizeof(benchmark_t), MPI_BYTE, i, t, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// !!! Attention: All pointers are invalid !!!
			bm[i]->processor = malloc(0);
			bm[i]->testfn = malloc(0);
			bm[i]->block = malloc(0);
			bm[i]->ms = malloc(0);
			// !!! END Attention: All pointers are invalid !!!

			// Get processor
			++t;
			MPI_Probe(i, t, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &count);
			bm[i]->processor = realloc(bm[i]->processor, count);
			MPI_Recv(bm[i]->processor, count, MPI_CHAR, i, t, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// Get test filename
			++t;
			MPI_Probe(i, t, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &count);
			bm[i]->testfn = realloc(bm[i]->testfn, count);
			MPI_Recv(bm[i]->testfn, count, MPI_CHAR, i, t, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// Get measurements
			++t;
			MPI_Probe(i, t, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_BYTE, &count);
			bm[i]->ms = realloc(bm[i]->ms, count);
			MPI_Recv(bm[i]->ms, count, MPI_BYTE, i, t, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
}


/**
 * @brief
 *
 * @param
 */
void report_init(report_t* report){
	report->bm = NULL;
}

/**
 * @brief
 *
 * @param report
 * @param bm
 */
void report_setup(report_t* report, benchmark_t* bm) {
	report->bm = bm;
}

/**
 * @brief
 *
 * @param report
 */
void report_destroy(report_t* report) {
	report->bm = NULL;
}



static void table_write_entry(
		table_t* table,
		const size_t nrows, const size_t ncols, const size_t pos,
		const char* name, const char* type, const char* quantity, const char* unit, const char* value) {
	assert(pos < ncols);
	table_t tab = *table;

	size_t nlen = strlen(name);
	tab[pos][0] = (char*)malloc(sizeof(char*) * nlen + 1);
	strcpy(tab[pos][0], name);

	size_t tlen = strlen(type);
	tab[pos][1] = (char*)malloc(sizeof(char*) * tlen + 1);
	strcpy(tab[pos][1], type);

	size_t qlen = strlen(quantity);
	tab[pos][2] = (char*)malloc(sizeof(char*) * qlen + 1);
	strcpy(tab[pos][2], quantity);

	size_t ulen = strlen(unit);
	tab[pos][3] = (char*)malloc(sizeof(char*) * ulen + 1);
	strcpy(tab[pos][3], unit);

	size_t vlen = strlen(value);
	tab[pos][4] = (char*)malloc(sizeof(char*) * vlen + 1);
	strcpy(tab[pos][4], value);
}





static void table_destroy(table_t* table, const size_t nrows, const size_t ncols) {
	table_t tab = *table;
	for (int row = 0; row < NROWS; ++row) {
		for (int col = 0; col < NCOLS; ++col) {
			free(tab[row][col]);
			tab[row][col] = NULL;
		}
	}
}

static void append_string(char** buf, size_t* buf_size, const char* vb) {
	assert(NULL != buf);
	assert(NULL != vb);
	assert(buf_size > 0);

	const size_t vb_len = strlen(vb);
	const size_t buf_len = strlen(*buf);
	size_t new_buf_size = 0;
	char* new_buf;
	while (vb_len + buf_len + 1 > *buf_size) {
		new_buf_size = *buf_size * 2;
		new_buf = (char*) malloc(sizeof(*new_buf) * new_buf_size);
		strcpy(new_buf, *buf);
		free(*buf);
		*buf_size = new_buf_size;
		*buf = new_buf;
	}
	snprintf(*buf + buf_len, *buf_size - buf_len, "%s", vb);
}



//typedef struct benchmark_t {
//	char* processor;
//	char* testfn;
//	char* block;
//	measurement_t* ms;
//	int nranks;
//	int rank;
//	size_t ndims;
//	size_t dgeom[NDIMS];
//	size_t bgeom[NDIMS];
//	size_t cgeom[NDIMS];
//	procs_t procs;
//	io_mode_t io_mode;
//	size_t block_size;
//	duration_t duration;
//	size_t mssize;
//	int par_access;
//	int storage;
//	bool is_unlimited;
//} benchmark_t;


static double sum(const double* elems, const size_t size){
	double sum = 0;
	for(size_t i = 0; i < size; ++i) {
		sum +=elems[i];
	}
	return sum;
}

static double avg(const double* elems, const size_t size) {
	return sum(elems, size) / size;
}

static double min(const double* elems, const size_t size) {
	assert(size > 0);
	double min = elems[0];
	for (size_t i = 0; i < size; ++i) {
		if (elems[i] < min) {
			min = elems[i];
		}
	}
	return min;
}

static double max(const double* elems, const size_t size) {
	assert(size > 0);
	double max = elems[0];
	for (size_t i = 0; i < size; ++i) {
		if (elems[i] > max) {
			max = elems[i];
		}
	}
	return max;
}


static double get_open_time(const benchmark_t* bms) {return bms->duration.open;}
static double get_io_time(const benchmark_t* bms) {return bms->duration.io;}
static double get_close_time(const benchmark_t* bms) {return bms->duration.close;}
static double get_perf(const benchmark_t* bm) {
	const size_t dsize = bm->dgeom[0] * bm->dgeom[1] * bm->dgeom[2] * bm->dgeom[3] * sizeof(bm->block[0]);
	const double total_time =  bm->duration.open + bm->duration.io + bm->duration.close;
	return dsize / total_time / (1024 * 1024);
}
static double get_perf_pure(const benchmark_t* bm) {
	const size_t dsize = bm->dgeom[0] * bm->dgeom[1] * bm->dgeom[2] * bm->dgeom[3] * sizeof(bm->block[0]);
	const double total_time =   bm->duration.io;
	return dsize / total_time / (1024 * 1024);
}


static double* create_bm_array (const benchmark_t** bms, size_t size, get_bm_value_t get_bm_value, double* array) {
	for (size_t i = 0; i < size; ++i) {
		array[i] = get_bm_value(bms[i]);
	}
	return array;
}

static void compute_stat (mam_t* mam, const benchmark_t** bms, const size_t bms_size, get_bm_value_t get_bm_value) {
	double* array = (double*) malloc(sizeof(double) * bms_size);
	mam->min = min(create_bm_array(bms, bms_size, get_bm_value, array), bms_size);
	mam->avg = avg(create_bm_array(bms, bms_size, get_bm_value, array), bms_size);
	mam->max = max(create_bm_array(bms, bms_size, get_bm_value, array), bms_size);
	free(array);
}

static void compute_stats (mam_t* open, mam_t* io, mam_t* close, mam_t* perf, mam_t* perf_pure, const benchmark_t** bms, const size_t bms_size) {
	compute_stat(open, bms, bms_size, get_open_time);
	compute_stat(io, bms, bms_size, get_io_time);
	compute_stat(close, bms, bms_size, get_close_time);
	compute_stat(perf, bms, bms_size, get_perf);
	compute_stat(perf_pure, bms, bms_size, get_perf_pure);
}





/**
 * @brief
 *
 * @param report
 * @param type
 */
void report_print(const report_t* report, const report_type_t type) {
	int nranks;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	benchmark_t** benchmarks = NULL;

  if (0 == rank){
    benchmarks= (benchmark_t**)malloc(sizeof(benchmark_t*) * nranks);
    for (int i = 1; i < nranks; ++i) {
      benchmarks[i] = malloc(sizeof(benchmark_t));
      benchmark_init(benchmarks[i]);
    }
    benchmarks[0] = report->bm;
  }

	benchmark_send(report->bm, 412);
	benchmark_receive(benchmarks, 412);

	if (0 == rank) {
		DEBUG_MESSAGE("REPORT_START\n");
		benchmark_t* bm = report->bm;
		char io_mode_str[10];
		switch (bm->io_mode) {
			case IO_MODE_WRITE:
				strcpy(io_mode_str, "write");
				break;
			case IO_MODE_READ:
				strcpy(io_mode_str, "read");
				break;
		}

		mam_t open_stats;
		mam_t io_stats;
		mam_t close_stats;
		mam_t perf_stats;
		mam_t perf_pure_stats;
		const size_t bms_size = benchmarks[0]->nranks;
		compute_stats(&open_stats, &io_stats, &close_stats, &perf_stats, &perf_pure_stats, (const benchmark_t**) benchmarks, bms_size);


		char id[200];
		sprintf(id, "%s:%s", "benchmark", io_mode_str);

		const int dist1 = 20;
		const int dist2 = 40;
		const int dist3 = 20;
		const int dist4 = 20;
		
		printf("%-*s %-*s %*.10s %*.10s %*.10s %-*s\n" , dist1 , "" , dist2 , ""                                 , dist3 , "min"               , dist3 , "avg"               , dist3 , "max"               , dist4 , "");
		printf("%-*s %-*s %*.10f %*.10f %*.10f %-*s\n" , dist1 , id , dist2 , "Open time"                        , dist3 , open_stats.min      , dist3 , open_stats.avg      , dist3 , open_stats.max      , dist4 , "secs");
		printf("%-*s %-*s %*.10f %*.10f %*.10f %-*s\n" , dist1 , id , dist2 , "I/O time"                         , dist3 , io_stats.min        , dist3 , io_stats.avg        , dist3 , io_stats.max        , dist4 , "secs");
		printf("%-*s %-*s %*.10f %*.10f %*.10f %-*s\n" , dist1 , id , dist2 , "Close time"                       , dist3 , close_stats.min     , dist3 , close_stats.avg     , dist3 , close_stats.max     , dist4 , "secs");
		printf("%-*s %-*s %*.10f %*.10f %*.10f %-*s\n" , dist1 , id , dist2 , "I/O Performance (w/o open/close)" , dist3 , perf_pure_stats.min , dist3 , perf_pure_stats.avg , dist3 , perf_pure_stats.max , dist4 , "MiB/s");
		printf("%-*s %-*s %*.10f %*.10f %*.10f %-*s\n" , dist1 , id , dist2 , "I/O Performance"                  , dist3 , perf_stats.min      , dist3 , perf_stats.avg      , dist3 , perf_stats.max      , dist4 , "MiB/s");

		DEBUG_MESSAGE("REPORT_END\n");
	}

  if (0 == rank) {
    for (int i = 1; i < nranks; ++i) {
      benchmark_destroy(benchmarks[i]);
      free(benchmarks[i]);
    }
    free(benchmarks);
  }
}
