#include <esdm-mpi-internal.h>


static void check_hash_abort(MPI_Comm com, int hash, int rank){
  int ret;
  int vals[] = {hash, -hash};

  if(rank == 0){
    int rcvd[2];
    ret = MPI_Reduce(vals, rcvd, 2, MPI_INT, MPI_MIN, 0, com);
    assert(ret == MPI_SUCCESS);

    if(rcvd[0] != hash || rcvd[1] != -hash){
      MPI_Abort(com, 0);
    }
  }else{
    ret = MPI_Reduce(vals, NULL, 2, MPI_INT, MPI_MIN, 0, com);
    assert(ret == MPI_SUCCESS);
  }
}

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
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  if(ret != MPI_SUCCESS) return ESDM_ERROR;
  if(rank == 0){
    ret = esdm_container_create(name, out_container);
    // TODO check existance and broadcast
  }else{
    ret = esdm_container_create(name, out_container);
  }
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
  assert(name != NULL);
  assert(container != NULL);
  assert(dataspace != NULL);

  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  assert(ret == MPI_SUCCESS);
  // compute hash to ensure that all arguments are the same
  int hash;
  hash = (ea_compute_hash_str(name)<<3) + dataspace->dims + ea_compute_hash_str(container->name);
  for(int i = dataspace->dims - 1 ; i >= 0 ; i--){
    hash = (hash<<1) + dataspace->size[i];
  }

  if(rank == 0){
    check_hash_abort(com, hash, 0);
    ret = esdm_dataset_create(container, name, dataspace, out_dataset);
  }else{
    check_hash_abort(com, hash, 1);
    esdm_dataset_init(container, name, dataspace, out_dataset);
  }

  return ret;
}

esdm_status esdm_mpi_dataset_retrieve(MPI_Comm com, esdm_container_t *container, const char *name, esdm_dataset_t **out_dataset){
  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  *out_dataset = NULL;

  assert(ret == MPI_SUCCESS);
  // compute hash to ensure all have the same value
  int hash;
  hash = ea_compute_hash_str(name) + ea_compute_hash_str(container->name);

  char * buff;
  int size;
  esdm_dataset_t *d;
  esdm_dataset_init(container, name, NULL, & d);

  if(rank == 0){
    check_hash_abort(com, hash, 0);
    ret = esdm_dataset_retrieve_md_load(d, & buff, & size);
  	if(ret != ESDM_SUCCESS){
  		free(d);
  		return ret;
  	}
    ret = MPI_Bcast(& size, 1, MPI_INT, 0, com);
    assert(ret == MPI_SUCCESS);

    ret = MPI_Bcast(buff, size, MPI_CHAR, 0, com);
    assert(ret == MPI_SUCCESS);
  }else{
    check_hash_abort(com, hash, 1);
    ret = MPI_Bcast(& size, 1, MPI_INT, 0, com);
    assert(ret == MPI_SUCCESS);

    buff = (char*) malloc(size);

    ret = MPI_Bcast(buff, size, MPI_CHAR, 0, com);
    assert(ret == MPI_SUCCESS);
  }

  ret = esdm_dataset_retrieve_md_parse(d, buff, size);
  free(buff);
  if(ret != ESDM_SUCCESS){
    free(d);
    return ret;
  }
  *out_dataset = d;
  return ESDM_SUCCESS;
}

esdm_status esdm_mpi_dataset_commit(MPI_Comm com, esdm_dataset_t *dataset){
  esdm_status ret;
  ret = esdm_dataset_commit(dataset);
  return ret;
}
