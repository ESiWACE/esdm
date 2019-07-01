#ifndef ESDM_MPI_H
#define ESDM_MPI_H

#include <mpi.h>

int esdm_mpi_get_tasks_per_node();

void esdm_mpi_distribute_config_file(char *config_filename);

void esdm_mpi_init();

void esdm_mpi_finalize();

#endif
