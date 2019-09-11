#ifndef ESDM_MPI_H
#define ESDM_MPI_H

#include <mpi.h>

#include <esdm.h>

int esdm_mpi_get_tasks_per_node();

void esdm_mpi_distribute_config_file(char *config_filename);

void esdm_mpi_init();

void esdm_mpi_finalize();


esdm_status esdm_mpi_container_create(MPI_Comm com, const char *name, int allow_overwrite, esdm_container_t **out_container);
esdm_status esdm_mpi_container_open(MPI_Comm com, const char *name, int allow_overwrite, esdm_container_t **out_container);

/* assumes that com is the same used for create/retrieve */
esdm_status esdm_mpi_container_commit(MPI_Comm com, esdm_container_t *container);


/* for a dataset, metadata can be added only at rank 0 */

esdm_status esdm_mpi_dataset_create(MPI_Comm com, esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset);
esdm_status esdm_mpi_dataset_open(MPI_Comm com, esdm_container_t *container, const char *name, esdm_dataset_t **out_dataset);
esdm_status esdm_mpi_dataset_ref(MPI_Comm com, esdm_dataset_t * d);
esdm_status esdm_mpi_dataset_commit(MPI_Comm com, esdm_dataset_t *dataset);


#endif
