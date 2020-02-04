#include <esdm-mpi-internal.h>


static void check_hash_abort(MPI_Comm com, int hash, int rank){
  int ret;
  int vals[] = {hash, -hash};

  if(rank == 0){
    int rcvd[2];
    ret = MPI_Reduce(vals, rcvd, 2, MPI_INT, MPI_MIN, 0, com);
    eassert(ret == MPI_SUCCESS);

    if(rcvd[0] != hash || rcvd[1] != -hash){
      MPI_Abort(com, 0);
    }
  }else{
    ret = MPI_Reduce(vals, NULL, 2, MPI_INT, MPI_MIN, 0, com);
    eassert(ret == MPI_SUCCESS);
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
    int ret = read_file(config_filename, &config);
    eassert(ret == 0);
    len = strlen(config) + 1;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
  } else {
    int len = 0;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    config = (char *)malloc(len);
    MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
  }
  esdm_load_config_str(config);
  free(config);
}

void esdm_mpi_init() {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  time_t timer;
  time(&timer);

  int pPerNode = esdm_mpi_get_tasks_per_node();
  esdm_set_procs_per_node(pPerNode);
  esdm_set_total_procs(size);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  srand(rank + (uint64_t) timer);
}

void esdm_mpi_finalize(){
  esdm_finalize();
}



esdm_status esdm_mpi_container_create(MPI_Comm com, const char *name, int allow_overwrite, esdm_container_t **out_container){
  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  if(ret != MPI_SUCCESS) return ESDM_ERROR;
  if(rank == 0){
    ret = esdm_container_create(name, allow_overwrite, out_container);
    int ret2 = MPI_Bcast(& ret, 1, MPI_INT, 0, com);
    eassert(ret2 == MPI_SUCCESS);
  }else{
    int ret2 = MPI_Bcast(& ret, 1, MPI_INT, 0, com);
    eassert(ret2 == MPI_SUCCESS);
    if(ret == MPI_SUCCESS){
      esdmI_container_init(name, out_container);
    }
  }
  return ret;
}

esdm_status esdm_mpi_container_open(MPI_Comm com, const char *name, int allow_overwrite, esdm_container_t **out_container){
  // TODO use allow overwrite
  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  if(ret != MPI_SUCCESS) return ESDM_ERROR;
  char * buff;
  int size;
  esdmI_container_init(name, out_container);
  esdm_container_t *c = *out_container;
  if(rank == 0){
    esdm_status ret = esdm_container_open_md_load(c, & buff, & size);
  	if(ret != ESDM_SUCCESS){
  		size = -1;
  	}

    size = size + 1; // broadcast string terminator as well
    ret = MPI_Bcast(& size, 1, MPI_INT, 0, com);
    eassert(ret == MPI_SUCCESS);
    if(size > 0){
      ret = MPI_Bcast(buff, size, MPI_CHAR, 0, com);
      eassert(ret == MPI_SUCCESS);
    }else{
  		esdmI_container_destroy(c);
  		return ESDM_ERROR;
    }
  }else{
    ret = MPI_Bcast(& size, 1, MPI_INT, 0, com);
    eassert(ret == MPI_SUCCESS);

    if(size == 0){
      esdmI_container_destroy(c);
      return ESDM_ERROR;
    }
    buff = (char*) malloc(size);

    ret = MPI_Bcast(buff, size, MPI_CHAR, 0, com);
    eassert(ret == MPI_SUCCESS);
  }
	ret = esdm_container_open_md_parse(c, buff, size);
	free(buff);
	if(ret != ESDM_SUCCESS){
		esdmI_container_destroy(c);
		return ret;
	}
  return ret;
}

esdm_status esdm_mpi_container_commit(MPI_Comm com, esdm_container_t *c){
  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  if(ret != MPI_SUCCESS) return ESDM_ERROR;

  esdm_datasets_t * dsets = & c->dsets;
  for(int i = 0; i < dsets->count; i++){
     esdm_mpi_dataset_commit(com, dsets->dset[i]);
  }

  if(rank == 0){
    ret = esdm_container_commit(c);
  }else{
    c->status = ESDM_DATA_PERSISTENT;
  }
  MPI_Bcast(& ret, 1, MPI_INT, 0, com);
  return ret;
}

esdm_status esdm_mpi_dataset_create(MPI_Comm com, esdm_container_t *c, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset){
  eassert(name != NULL);
  eassert(c != NULL);
  eassert(dataspace != NULL);

  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  eassert(ret == MPI_SUCCESS);
  // compute hash to ensure that all arguments are the same
  int hash;
  hash = (ea_compute_hash_str(name)<<3) + dataspace->dims + ea_compute_hash_str(c->name);
  for(int i = dataspace->dims - 1 ; i >= 0 ; i--){
    hash = (hash<<1) + dataspace->size[i];
  }

  if(rank == 0){
    check_hash_abort(com, hash, 0);
    ret = esdm_dataset_create(c, name, dataspace, out_dataset);
    char * id = "";
    if(ret == ESDM_SUCCESS){
      id = (*out_dataset)->id;
    }
    int ret2 = MPI_Bcast(id, strlen(id)+1, MPI_CHAR, 0, com);
    eassert(ret2 == MPI_SUCCESS);
    return ret;
  }else{
    check_hash_abort(com, hash, 1);

    char id[20];
    ret = MPI_Bcast(id, 17, MPI_CHAR, 0, com);
    eassert(ret == MPI_SUCCESS);
    if(strlen(id) == 0){
      return ESDM_ERROR;
    }

    esdm_dataset_init(c, name, dataspace, out_dataset);
    (*out_dataset)->id = strdup(id);
    esdmI_container_register_dataset(c, *out_dataset);

    return ESDM_SUCCESS;
  }
}

esdm_status esdm_mpi_dataset_ref(MPI_Comm com, esdm_dataset_t * d){
  ESDM_DEBUG(__func__);
  assert(d);
  if(d->status != ESDM_DATA_NOT_LOADED){
    d->refcount++;
    return ESDM_SUCCESS;
  }

  char * buff;
  int size;
  int rank;
  int ret = MPI_Comm_rank(com, & rank);
  eassert(ret == MPI_SUCCESS);

  if(rank == 0){
    ret = esdm_dataset_open_md_load(d, & buff, & size);
  	if(ret != ESDM_SUCCESS){
  		size = -1;
  	}
    size = size + 1; // broadcast string terminator as well
    ret = MPI_Bcast(& size, 1, MPI_INT, 0, com);
    eassert(ret == MPI_SUCCESS);

    if(size > 0){
      ret = MPI_Bcast(buff, size, MPI_CHAR, 0, com);
      eassert(ret == MPI_SUCCESS);
    }else{
  		return ret;
    }
  }else{
    ret = MPI_Bcast(& size, 1, MPI_INT, 0, com);
    eassert(ret == MPI_SUCCESS);
    if (size == 0){
      return ESDM_ERROR;
    }

    buff = (char*) malloc(size);

    ret = MPI_Bcast(buff, size, MPI_CHAR, 0, com);
    eassert(ret == MPI_SUCCESS);
  }
  ret = esdm_dataset_open_md_parse(d, buff, size);
  if(ret != ESDM_SUCCESS){
    return ret;
  }

  free(buff);
  d->refcount++;
  return ESDM_SUCCESS;
}

esdm_status esdm_mpi_dataset_open(MPI_Comm com, esdm_container_t *c, const char *name, esdm_dataset_t **out_dataset){
  // compute hash to ensure all have the same value
  //int hash;
  //hash = ea_compute_hash_str(name) + ea_compute_hash_str(c->name);
  esdm_dataset_t *d = NULL;
  esdm_datasets_t * dsets = & c->dsets;
  for(int i=0; i < dsets->count; i++ ){
    if(strcmp(dsets->dset[i]->name, name) == 0){
      d = dsets->dset[i];
      *out_dataset = d;
      return esdm_mpi_dataset_ref(com, d);
    }
  }
  return ESDM_ERROR;
}

esdm_status esdm_mpi_dataset_commit(MPI_Comm com, esdm_dataset_t *d){
  esdm_status ret;
  int rank;
  ret = MPI_Comm_rank(com, & rank);
  //if(rank != 0 && d->attr->childs != 0){
  //  ESDM_ERROR("Only Rank 0 can attach metadata to a dataset");
  //  return ESDM_ERROR;
  //}

  // retrieve for all fragments the metadata and attach it to the metadata
  if(rank == 0){
    int procCount;
    ret = MPI_Comm_size(com, & procCount);
    int max_len = 10000000;
    char * buff = malloc(max_len);
    for(int p = 1 ; p < procCount; p++){
      int size = max_len;
      MPI_Status mstatus;
      ret = MPI_Recv(buff, size, MPI_CHAR, MPI_ANY_SOURCE, 4711, com, & mstatus);
      eassert(ret == MPI_SUCCESS);
      int recvd_bytes = 0;
      ret = MPI_Get_count(& mstatus, MPI_CHAR, & recvd_bytes);
      eassert(ret == MPI_SUCCESS);
      if (buff[0] != '[' || buff[recvd_bytes-2] != ']' || buff[recvd_bytes -1] != 0){
        ESDM_ERROR_FMT("Buffer appears to be corrupted (%d bytes) \"%s\"\n", recvd_bytes, buff);
      }

      json_t * elem = load_json(buff);
      int json_frags = json_array_size(elem);

      for (int i = 0; i < json_frags; i++) {
        esdm_fragment_t * fragment;
        json_t * frag_elem = json_array_get(elem, i);
        ret = esdmI_create_fragment_from_metadata(d, frag_elem, & fragment);
        if (ret != ESDM_SUCCESS){
          MPI_Abort(com, 1);
        }
        esdmI_fragments_add(d->fragments, fragment);
      }
      json_decref(elem);
    }
    free(buff);
    ret = esdm_dataset_commit(d);

    MPI_Bcast(& ret, 1, MPI_INT, 0, com);
    return ret;
  }else{
    size_t size;
    smd_string_stream_t * s = smd_string_stream_create();
    esdmI_fragments_metadata_create(d->fragments, s);
    char * buff = smd_string_stream_close(s, & size);

    ret = MPI_Send(buff, size + 1, MPI_CHAR, 0, 4711, com);
    eassert(ret == MPI_SUCCESS);
    free(buff);

    d->status = ESDM_DATA_PERSISTENT;
    MPI_Bcast(& ret, 1, MPI_INT, 0, com);

    return ret;
  }
}
