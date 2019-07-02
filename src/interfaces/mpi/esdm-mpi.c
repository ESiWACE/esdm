#include <esdm-mpi-internal.h>

int esdm_mpi_get_tasks_per_node() {
  MPI_Comm shared_comm;
  int count = 1;

  MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shared_comm);
  MPI_Comm_size(shared_comm, &count);
  MPI_Comm_free(&shared_comm);

  return count;
}

void esdm_mpi_distribute_config_file(char *config_filename) {
  int mpi_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  char *config = NULL;
  if (mpi_rank == 0) {
    int len;
    read_file(config_filename, &config);
    len = strlen(config) + 1;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
  } else {
    int len;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    config = (char *)malloc(len);
    MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
  }
  esdm_load_config_str(config);
}

void esdm_mpi_init() {
  int mpi_size;
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  int pPerNode = esdm_mpi_get_tasks_per_node();
  esdm_set_procs_per_node(pPerNode);
  esdm_set_total_procs(mpi_size);
}

void esdm_mpi_finalize(){
  esdm_finalize();
}



esdm_status esdm_mpi_container_create(MPI_Comm com, const char *name, esdm_container_t **out_container){
  esdm_status ret;
  ret = esdm_container_create(name, out_container);
  return ret;
}

esdm_status esdm_mpi_container_retrieve(MPI_Comm com, const char *name, esdm_container_t **out_container){
  esdm_status ret;
  ret = esdm_container_retrieve(name, out_container);
  return ret;
}

esdm_status esdm_mpi_container_commit(MPI_Comm com, esdm_container_t *container){
  esdm_status ret;
  ret = esdm_container_commit(container);
  return ret;
}



esdm_status esdm_mpi_dataset_create(MPI_Comm com, esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset){
  esdm_status ret;
  ret = esdm_dataset_create(container, name, dataspace, out_dataset);
  return ret;
}

esdm_status esdm_mpi_dataset_retrieve(MPI_Comm com, esdm_container_t *container, const char *name, esdm_dataset_t **out_dataset){
  esdm_status ret;
  ret = esdm_dataset_retrieve(container, name, out_dataset);
  return ret;
}

esdm_status esdm_mpi_dataset_commit(MPI_Comm com, esdm_dataset_t *dataset){
  esdm_status ret;
  ret = esdm_dataset_commit(dataset);
  return ret;
}
