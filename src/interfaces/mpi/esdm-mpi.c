#include <esdm-mpi.h>

#include <esdm-debug.h>
#include <esdm-grid.h>
#include <esdm-internal.h>


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

esdm_status esdm_mpi_distribute_config_file(char *config_filename) {
  int mpi_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  char *config = NULL;
  if (mpi_rank == 0) {
    int len;
    int ret = ea_read_file(config_filename, &config);
    if(ret == 1){
      return ESDM_ERROR;
    }
    eassert(ret == 0);
    len = strlen(config) + 1;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
  } else {
    int len = 0;
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    config = ea_checked_malloc(len);
    MPI_Bcast(config, len, MPI_CHAR, 0, MPI_COMM_WORLD);
  }
  esdm_load_config_str(config);
  free(config);

  return ESDM_SUCCESS;
}



esdm_status esdm_mpi_init_manual() {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  time_t timer;
  time(&timer);

  int pPerNode = esdm_mpi_get_tasks_per_node();
  esdm_set_procs_per_node(pPerNode);
  esdm_set_total_procs(size);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  srand(rank + (uint64_t) timer); //FIXME: Don't use [s]rand().

  return ESDM_SUCCESS;
}

esdm_status esdm_mpi_init() {
  esdm_mpi_init_manual();
  int esdm_status = esdm_mpi_distribute_config_file("esdm.conf");
  if(esdm_status == ESDM_ERROR){
    return ESDM_ERROR;
  }
  return esdm_init();
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
    buff = ea_checked_malloc(size);

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
    size_t length = strlen(id);
    eassert(length <= ESDM_ID_LENGTH);
    int ret2 = MPI_Bcast(id, length + 1, MPI_CHAR, 0, com);
    eassert(ret2 == MPI_SUCCESS);
    return ret;
  }else{
    check_hash_abort(com, hash, 1);

    char id[ESDM_ID_LENGTH + 1];
    ret = MPI_Bcast(id, ESDM_ID_LENGTH + 1, MPI_CHAR, 0, com);
    eassert(ret == MPI_SUCCESS);
    if(strlen(id) == 0){
      return ESDM_ERROR;
    }

    esdm_dataset_init(c, name, dataspace, out_dataset);
    (*out_dataset)->id = ea_checked_strdup(id);
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

    buff = ea_checked_malloc(size);

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
    char * buff = ea_checked_malloc(max_len);
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
        esdmI_fragments_add(&d->fragments, fragment);
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
    esdmI_fragments_metadata_create(&d->fragments, s);
    char * buff = smd_string_stream_close(s, & size);

    ret = MPI_Send(buff, size + 1, MPI_CHAR, 0, 4711, com);
    eassert(ret == MPI_SUCCESS);
    free(buff);

    MPI_Bcast(& ret, 1, MPI_INT, 0, com);

    return ret;
  }
}

__attribute__((noreturn))
void panic(const char* operation) {
  fprintf(stderr, "failed %s, aborting...\n", operation);
  abort();
}

esdm_status esdm_mpi_grid_bcast(MPI_Comm comm, esdm_dataset_t* dataset, esdm_grid_t** inout_grid) {
  eassert(dataset);
  eassert(dataset->id);
  eassert(inout_grid);

  int rank;
  if(MPI_SUCCESS != MPI_Comm_rank(comm, &rank)) return ESDM_ERROR;
  int localResult = ESDM_SUCCESS;

  if(!rank) {
    eassert(*inout_grid && "inout_grid is an input parameter at the root process");

    uint64_t datasetIdSize = strlen(dataset->id);
    if(MPI_SUCCESS != MPI_Bcast(&datasetIdSize, 1, MPI_UINT64_T, 0, comm)) panic("MPI_Bcast");
    if(MPI_SUCCESS != MPI_Bcast(dataset->id, datasetIdSize + 1, MPI_BYTE, 0, comm)) panic("MPI_Bcast");

    smd_string_stream_t* stream = smd_string_stream_create();
    if(!stream) panic("memory allocation");
    esdmI_grid_serialize(stream, *inout_grid);
    size_t size;
    char* serializedGrid = smd_string_stream_close(stream, &size);

    uint64_t mpiSize = size;  //MPI does not have a type for `size_t`
    if(MPI_SUCCESS != MPI_Bcast(&mpiSize, 1, MPI_UINT64_T, 0, comm)) panic("MPI_Bcast");
    if(MPI_SUCCESS != MPI_Bcast(serializedGrid, mpiSize + 1, MPI_BYTE, 0, comm)) panic("MPI_Bcast");
    free(serializedGrid);
  } else {
    uint64_t rootDatasetIdSize;
    if(MPI_SUCCESS != MPI_Bcast(&rootDatasetIdSize, 1, MPI_UINT64_T, 0, comm)) panic("MPI_Bcast");
    char* rootDatasetId = ea_checked_malloc(rootDatasetIdSize + 1);
    if(MPI_SUCCESS != MPI_Bcast(rootDatasetId, rootDatasetIdSize + 1, MPI_BYTE, 0, comm)) panic("MPI_Bcast");
    bool datasetIdMatches = !strcmp(rootDatasetId, dataset->id);
    free(rootDatasetId);

    uint64_t stringSize;
    if(MPI_SUCCESS != MPI_Bcast(&stringSize, 1, MPI_UINT64_T, 0, comm)) panic("MPI_Bcast");
    char* string = ea_checked_malloc(stringSize + 1);
    if(MPI_SUCCESS != MPI_Bcast(string, stringSize + 1, MPI_BYTE, 0, comm)) panic("MPI_Bcast");

    //TODO: Check whether we already have a grid with this ID, and merge the data into that grid in that case.
    //      This requires that we know about the corresponding dataset here.
    localResult = datasetIdMatches ? esdmI_grid_createFromString(string, dataset, inout_grid) : ESDM_INVALID_ARGUMENT_ERROR;
    free(string);
  }

  int globalResult;
  if(MPI_SUCCESS != MPI_Allreduce(&localResult, &globalResult, 1, MPI_INT, MPI_MAX, comm)) panic("MPI_Allreduce");
  return globalResult;
}

esdm_status esdm_mpi_grid_commit(MPI_Comm comm, esdm_grid_t* grid) {
  eassert(grid);

  int rank, procCount;
  if(MPI_SUCCESS != MPI_Comm_rank(comm, &rank)) return ESDM_ERROR;
  if(MPI_SUCCESS != MPI_Comm_size(comm, &procCount)) return ESDM_ERROR;
  int result = ESDM_SUCCESS;

  if(!rank) {
    uint64_t* sizes = ea_checked_malloc(procCount*sizeof*sizes), dummy = 0;
    if(MPI_SUCCESS != MPI_Gather(&dummy, 1, MPI_UINT64_T, sizes, 1, MPI_UINT64_T, 0, comm)) panic("MPI_Gather");

    int* counts = ea_checked_malloc(procCount*sizeof*counts);
    int* offsets = ea_checked_malloc(procCount*sizeof*offsets);
    offsets[0] = counts[0] = 0;
    for(int proc = 1; proc < procCount; proc++) {
      counts[proc] = sizes[proc];
      eassert(counts[proc] == sizes[proc] && "`int` must be large enough to hold the sizes of the serialized grids");
      offsets[proc] = offsets[proc - 1] + counts[proc - 1];
      eassert(INT_MAX - counts[proc] > offsets[proc] && "`int` must be large enough to hold the total size of the serialized grids");
    }
    int totalSize = offsets[procCount - 1] + counts[procCount - 1];
    char* serializedGrids = ea_checked_malloc(totalSize*sizeof*serializedGrids);
    if(MPI_SUCCESS != MPI_Gatherv(&dummy, 0, MPI_BYTE, serializedGrids, counts, offsets, MPI_BYTE, 0, comm)) panic("MPI_Gatherv");

    for(int proc = 1; proc < procCount; proc++) {
      result = esdmI_grid_mergeWithString(grid, serializedGrids + offsets[proc]);
      if(result != ESDM_SUCCESS) break;
    }

    free(serializedGrids);
    free(offsets);
    free(counts);
    free(sizes);
  } else {
    smd_string_stream_t* stream = smd_string_stream_create();
    if(!stream) panic("memory allocation");
    esdmI_grid_serialize(stream, grid);
    size_t size;
    char* serializedGrid = smd_string_stream_close(stream, &size);

    uint64_t mpiSize = size + 1, dummy;  //MPI does not have a type for `size_t`, +1 because we want to send the terminating null byte as well
    if(MPI_SUCCESS != MPI_Gather(&mpiSize, 1, MPI_UINT64_T, &dummy, 1, MPI_UINT64_T, 0, comm)) panic("MPI_Gather");
    if(MPI_SUCCESS != MPI_Gatherv(serializedGrid, mpiSize, MPI_BYTE, &dummy, NULL, NULL, MPI_BYTE, 0, comm)) panic("MPI_Gatherv");

    free(serializedGrid);
  }

  if(MPI_SUCCESS != MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD)) panic("MPI_Bcast");
  return result;
}
