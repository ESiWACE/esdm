/*
 * =====================================================================================
 *
 *       Filename:  debug.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/15/2016 10:45:18 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */

#include <libgen.h>
#include <mpi.h>

#ifdef DEBUG
#define DEBUG_MESSAGE(...) \
{ \
	int debug_rank = 0; \
 	MPI_Comm_rank(MPI_COMM_WORLD, &debug_rank); \
	fprintf(stdout, "DEBUG [%d] %*s:%-*d ", debug_rank, 15, basename(__FILE__), 5, __LINE__); \
	fprintf(stdout, __VA_ARGS__); \
}
#else
#define DEBUG_MESSAGE(...)
#endif

#ifndef FATAL_ERR
#define FATAL_ERR(...) \
	fprintf(stderr, "Error in %s:%d:%s: ", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
	fprintf(stderr, __VA_ARGS__); \
	exit(-1)

#endif
