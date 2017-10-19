/*
 * =====================================================================================
 *
 *       Filename:  debug.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/20/2017 08:35:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke, Olga Perevalova
 *   Organization:  
 *
 * =====================================================================================
 */


#ifndef  debug_INC
#define  debug_INC

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <mpi.h>

#define LOG 520
// debug
#if TRACE
#define TRACEMSG(...) \
{\
	int debug_mpi_rank = 0; \
MPI_Comm_rank(MPI_COMM_WORLD, &debug_mpi_rank); \
printf("[TRACE] (%d) %s:%d - ", debug_mpi_rank, __PRETTY_FUNCTION__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n"); \
}
#else
#define TRACEMSG(...)
#endif


// debug
#if DEBUG
#define DEBUGMSG(...) \
{\
	int debug_mpi_rank = 0; \
MPI_Comm_rank(MPI_COMM_WORLD, &debug_mpi_rank); \
printf("[DEBUG] (%d) %s:%d - ", debug_mpi_rank, __PRETTY_FUNCTION__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n"); \
}
#else
#define DEBUGMSG(...)
#endif


// error
#define ERRORMSG(...) \
{\
	int debug_mpi_rank = 0; \
MPI_Comm_rank(MPI_COMM_WORLD, &debug_mpi_rank); \
printf("[ERROR] (%d) %s:%d - ", debug_mpi_rank, __PRETTY_FUNCTION__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n"); \
assert(false); \
exit(1); \
}


// todo
#define TODOMSG(...) \
{\
	int debug_mpi_rank = 0; \
MPI_Comm_rank(MPI_COMM_WORLD, &debug_mpi_rank); \
printf("[TODO] (%d) %s:%d - ", debug_mpi_rank, __PRETTY_FUNCTION__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n"); \
}

#endif   /* ----- #ifndef debug_INC  ----- */
