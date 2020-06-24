#include <esdm-stream.h>

#include <esdm.h>
#include <esdm-datatypes-internal.h>
#include <esdm-debug.h>
#include <esdm-internal.h>

struct esdm_wstream_metadata_t {
  //data description
  esdm_dataset_t* dataset;
  esdm_dataspace_t* dataspace;
  esdm_backend_t* backend;
  estream_write_t backendState;

  //fragmentation/chunking parameters
  int64_t fragmentationDim;
  int64_t chunkingDim, maxChunkWidth; //maxChunkWidth controls how `chunkCounts[chunkingDim]` is calculated
  int64_t* fragmentCounts, *chunkCounts;  //For each dimension, this gives the count of fragments/chunks within that dimension.
                                          //In the case that `fragmentationDim == chunkingDim`,
                                          //`chunkCounts[chunkingDim]` gives only the count of chunks within the current fragment,
                                          //and will be recalculated for every fragment that is generated.
  int64_t* cumulativeFragmentCounts, *cumulativeChunkCounts;  //one longer than `dataspace->dims`, `cumulativeCounts[i]` is the product of all counts `>= i`,
                                                              //i.e., `cumulativeCounts[dims] == 1`, `cumulativeCount[dims-1] == counts[dims-1]`, and so on.

  //iterator status
  int64_t curFragment, nextChunk;
  int64_t chunkOffset;
};

static const int64_t kMaxFragmentSize = 16*1024*1024;  //TODO: This should come from configuration.
static const int64_t kMaxChunkSize = 16*1024;  //TODO: This should come from configuration.

//returns the max. object width
static void initCounts(int64_t dimCount, int64_t elementSize, int64_t maxObjectSize, int64_t* dataspaceSize, int64_t* out_splittingDim, int64_t* out_maxWidth, int64_t* objectCounts, int64_t* cumulativeCounts) {
  int64_t i = dimCount - 1;
  for(int64_t sliceSize = elementSize; i >= 0; i--, sliceSize *= dataspaceSize[i]) {
    int64_t maxSlices = maxObjectSize/sliceSize;
    if(maxSlices < dataspaceSize[i]) {
      objectCounts[i] = (dataspaceSize[i] + maxSlices - 1)/maxSlices;
      *out_splittingDim = i;
      if(out_maxWidth) *out_maxWidth = maxSlices;
      break;
    }
    objectCounts[i] = 1;
  }
  while(i-- > 0) objectCounts[i] = dataspaceSize[i];

  cumulativeCounts[dimCount] = 1;
  for(i = dimCount; i-- > 0; ) cumulativeCounts[i] = objectCounts[i]*cumulativeCounts[i + 1];
}

esdm_wstream_metadata_t* esdm_wstream_metadata_create(esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size, esdm_type_t type) {
  eassert(dataset->dataspace->dims == dimCount);

  esdm_wstream_metadata_t* result = ea_checked_malloc(sizeof*result);
  *result = (esdm_wstream_metadata_t){
    .dataset = dataset,
    .backend = esdm_modules_fastestBackend(esdm_get_modules()),
    .backendState = {0},

    //these are the defaults for the case that the entire stream region fits into a single fragment/chunk
    .fragmentationDim = 0,
    .chunkingDim = 0,
    .maxChunkWidth = dimCount ? size[0] : 1,
    .fragmentCounts = ea_checked_malloc(dimCount*sizeof*result->fragmentCounts),
    .chunkCounts = ea_checked_malloc(dimCount*sizeof*result->chunkCounts),
    .cumulativeFragmentCounts = ea_checked_malloc((dimCount + 1)*sizeof*result->cumulativeFragmentCounts),
    .cumulativeChunkCounts = ea_checked_malloc((dimCount + 1)*sizeof*result->cumulativeChunkCounts),

    .curFragment = 0,
    .nextChunk = 0,
    .chunkOffset = 0
  };
  eassert(result->backend);
  esdm_status ret = esdm_dataspace_create_full(dimCount, size, offset, type, &result->dataspace);
  if(ret != ESDM_SUCCESS) {
    ESDM_WARN("could not create dataspace");
    free(result);
    return NULL;
  }

  //Find the parameters for splitting the dataspace into fragments, and splitting fragments into chunks.
  initCounts(dimCount, esdm_sizeof(type), kMaxChunkSize, size, &result->chunkingDim, &result->maxChunkWidth, result->chunkCounts, result->cumulativeChunkCounts);
  initCounts(dimCount, esdm_sizeof(type), kMaxFragmentSize, size, &result->fragmentationDim, NULL, result->fragmentCounts, result->cumulativeFragmentCounts);

  return result;
}

static esdmI_range_t getBounds(esdm_dataspace_t* dataspace, int64_t dim, int64_t* cumulativeCounts, int64_t objectIndex) {
  if(!dataspace->dims) return (esdmI_range_t){ .start = 0, .end = 1 };  //avoid UB when dealing with a scalar dataspace

  int64_t dimSize = cumulativeCounts[dim]/cumulativeCounts[dim + 1];
  int64_t index = objectIndex / cumulativeCounts[dim + 1] % dimSize; //the one-dim index of the object
  return (esdmI_range_t){
    .start = dataspace->offset[dim] + index*dataspace->size[dim]/dimSize,
    .end = dataspace->offset[dim] + (index + 1)*dataspace->size[dim]/dimSize
  };
}

int64_t esdm_wstream_metadata_max_chunk_size(esdm_wstream_metadata_t* metadata) {
  //compute the size of each slice at the chunking dimension
  int64_t sliceSize = 1;
  for(int64_t dim = metadata->dataspace->dims; dim-- > metadata->chunkingDim; sliceSize *= metadata->dataspace->size[dim]) ;

  return metadata->maxChunkWidth * sliceSize;
}

static bool isFinished(esdm_wstream_metadata_t* metadata) {
  return metadata->curFragment == metadata->cumulativeFragmentCounts[0];
}

int64_t esdm_wstream_metadata_next_chunk_size(esdm_wstream_metadata_t* metadata) {
  if(isFinished(metadata)) return 0;  //signal that we expect no more data

  int64_t chunkSize = 1;
  for(int64_t i = metadata->dataspace->dims; i--; ) {
    chunkSize *= esdmI_range_size(getBounds(metadata->dataspace, i, metadata->cumulativeChunkCounts, metadata->nextChunk));
  }
  return chunkSize;
}

void esdm_wstream_flush(esdm_wstream_metadata_t* metadata, void* buffer, void* bufferEnd) {
  eassert(!isFinished(metadata));

  //check whether we need to create another fragment
  int64_t dimCount = metadata->dataspace->dims;
  if(!metadata->nextChunk) {
    eassert(!metadata->backendState.fragment);

    int64_t offset[dimCount], size[dimCount];
    for(int64_t dim = dimCount; dim--; ) {
      esdmI_range_t range = getBounds(metadata->dataspace, dim, metadata->cumulativeFragmentCounts, metadata->curFragment);
      offset[dim] = range.start;
      size[dim] = range.end - range.start;
    }
    esdm_dataspace_t* memspace;
    esdm_status ret = esdm_dataspace_create_full(dimCount, size, offset, metadata->dataspace->type, &memspace);
    if(ret != ESDM_SUCCESS) {
      fprintf(stderr, "esdm_wstream_flush(): error creating dataspace for fragment\nWhat dataspace/dataset was passed to esdm_wstream_start()?\naborting\n");
      abort();
    }
    ret = esdmI_fragment_create(metadata->dataset, memspace, NULL, &metadata->backendState.fragment);
    if(ret != ESDM_SUCCESS) {
      fprintf(stderr, "esdm_wstream_flush(): error creating fragment\nWhat dataspace/dataset was passed to esdm_wstream_start()?\naborting\n");
      abort();
    }
    metadata->backendState.fragment->backend = metadata->backend;
  }

  int64_t curChunkSize = (char*)bufferEnd - (char*)buffer;
  int ret = esdmI_backend_fragment_write_stream_blocksize(metadata->backend, &metadata->backendState, buffer, metadata->chunkOffset, curChunkSize);
  if(ret != ESDM_SUCCESS) {
    //TODO: Handle this error condition
    fprintf(stderr, "backend returned an error while flushing data from a write stream\naborting...\n");
    abort();
  }

  //advance the iterator status to the next chunk
  metadata->chunkOffset += curChunkSize;
  if(++(metadata->nextChunk) == metadata->cumulativeChunkCounts[0]) {
    if(metadata->chunkOffset != esdm_dataspace_total_bytes(metadata->backendState.fragment->dataspace)) {
      fprintf(stderr, "contract violation in esdm_wstream_flush() call:\nThe size of the streamed data does not match with the expected size.\nThis is either the result of a direct use of esdm_wstream_flush() by user code, or a library version mismatch between the shared object file and the esdm-stream.h header file used. Please ensure that user code only uses the preprocessor macros supplied by the linked library version to interact with streams.\naborting...\n");
      abort();
    }
    metadata->chunkOffset = 0;
    metadata->nextChunk = 0;
    metadata->curFragment++;

    metadata->backendState.fragment = NULL;

    if(isFinished(metadata)) return;  //all data has been streamed, nothing more to do

    //recalculate the chunk count for the chunking dimension
    if(dimCount) { //avoid UB in the case of a scalar dataspace
      int64_t fragmentWidth = esdmI_range_size(getBounds(metadata->dataspace, metadata->chunkingDim, metadata->cumulativeFragmentCounts, metadata->curFragment));
      metadata->chunkCounts[metadata->chunkingDim] = (fragmentWidth + metadata->maxChunkWidth - 1)/metadata->maxChunkWidth;
      for(int64_t i = metadata->chunkingDim; i >= 0; i--) metadata->cumulativeChunkCounts[i] = metadata->chunkCounts[i] * metadata->cumulativeChunkCounts[i + 1];
    }
  }
}

void esdm_wstream_metadata_destroy(esdm_wstream_metadata_t* metadata) {
  eassert(isFinished(metadata));

  esdm_dataspace_destroy(metadata->dataspace);
  free(metadata->fragmentCounts);
  free(metadata->chunkCounts);
  free(metadata->cumulativeFragmentCounts);
  free(metadata->cumulativeChunkCounts);
  free(metadata);
}
