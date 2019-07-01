#include <esdm-mpi.h>
#include <esdm-internal.h>


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
