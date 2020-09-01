#ifndef ESDM_MPI_INTERNAL_H
#define ESDM_MPI_INTERNAL_H

#include <esdm-mpi.h>
#include <esdm-internal.h>

/**
 * esdmI_mpi_grid_collect()
 *
 * Merge the information about all fragments that have been added to the grid copies after an `esdm_mpi_grid_bcast()`.
 *
 * @param comm the MPI communicator that defines the process set, must be the same as the one used in the preceding `esdm_mpi_grid_bcast()` call
 * @param grid the local copy of a grid that was previously broadcasted with `esdm_mpi_grid_bcast()`
 */
esdm_status esdmI_mpi_grid_collect(MPI_Comm comm, esdm_grid_t* grid);

#endif
