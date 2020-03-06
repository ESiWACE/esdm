#define _GNU_SOURCE

#include <esdm-internal.h>
#include <math.h>
#include <stdlib.h>
#include <test/util/test_util.h>

const int64_t gMaxBlockSize = 1024*1024;

static struct esdm_fragments_vtable_t gVtable;  //tentative definition, actual definition is at the end of the file

static double root(double degree, double radicant) {
  return exp(log(radicant)/degree);
}

static void ensureInitialization(struct esdmI_regularFragments_t* me) {
  if(me->dimCount >= 0) return; //fast return if we are already initialized

  //precondition for initialization
  esdm_dataspace_t* dataspace = me->super.parent->dataspace;
  eassert(dataspace && "cannot initialize without knowing the extends of the dataset");

  //some raw data and initialization
  esdmI_hypercube_t* extends;
  if(esdmI_dataspace_getExtends(dataspace, &extends) != ESDM_SUCCESS) ESDM_ERROR("failed to get the extends of the dataspace");
  int64_t dims = esdmI_hypercube_dimensions(extends);
  int64_t extendsSize = esdmI_hypercube_size(extends);
  me->dimCount = dims,
  me->fragmentSize = malloc(dims*sizeof(*me->fragmentSize)),
  me->fragmentOffset = malloc(dims*sizeof(*me->fragmentOffset)),
  me->fragmentCount = malloc(dims*sizeof(*me->fragmentCount)),
  esdmI_hypercube_getOffsetAndSize(extends, me->fragmentOffset, me->fragmentSize);

  //Now the interesting part: Find a suitable fragmentSize.
  eassert(dims >= 1);
  bool dimIsSingleRange[dims];
  memset(dimIsSingleRange, 0, sizeof(dimIsSingleRange));
  //we basically initialize these variables with the result of the first loop iteration (i = 0) in order to define a suitable `endIndex`
  int64_t singleRangeDimCount = dimIsSingleRange[0] = me->fragmentSize[0] <= root(dims, gMaxBlockSize);
  int64_t singleRangeSize = dimIsSingleRange[0] ? me->fragmentSize[0] : 1;
  for(int64_t i = 1, endIndex = 0; i != endIndex && dims - singleRangeDimCount >= 1; i = (i+1)%dims) {
    if(dimIsSingleRange[i]) continue;
    //Check the fragment size against the length that would provide fragments of gMaxBlockSize if it was used for all splitable dimensions.
    if(me->fragmentSize[i] <= root(dims - singleRangeDimCount, gMaxBlockSize/(double)singleRangeSize)) {
      //this fragment is too small to split, make a note and adjust the expected size for the remaining dimensions
      dimIsSingleRange[i] = true;
      singleRangeDimCount++;
      singleRangeSize *= me->fragmentSize[i];
      endIndex = i; //reconsider all other dimensions since we have increased our size expectation for them
    }
  }
  //actually compute the size of the fragments and their count in each dimension
  if(dims > singleRangeDimCount) {
    int64_t maxDimLength = root(dims - singleRangeDimCount, gMaxBlockSize/(double)singleRangeSize);
    me->totalFragmentCount = 1;
    for(int64_t i = 0; i < dims; i++) {
      me->fragmentCount[i] = (me->fragmentSize[i] + maxDimLength - 1)/maxDimLength; //ciel(me->fragmentSize[i]/maxDimLength)
      me->fragmentSize[i] = (me->fragmentSize[i] + me->fragmentCount[i] - 1)/me->fragmentCount[i];  //ciel(me->fragmentSize[i]/me->fragmentCount[i])
      me->totalFragmentCount *= me->fragmentCount[i];
    }
  } else {
    me->totalFragmentCount = 1;
    for(int64_t i = 0; i < dims; i++) me->fragmentCount[i] = 1;
  }

  //initialize the multidimensional array of fragments
  me->fragments = calloc(me->totalFragmentCount, sizeof(*me->fragments));
}

void esdmI_regularFragments_construct(struct esdmI_regularFragments_t* me) {
  //Since the dataset may not be associated with a dataspace yet, and since we need the dataspace to know its extends, which we need to compute the binning parameters,
  //we are doing a lazy initialization here:
  //We simply set our vtable and initalize the dimCount to -1 to signal that initialization is still neede.
  me->super.vtable = &gVtable;
  me->dimCount = -1;
  if(me->super.parent->dataspace) ensureInitialization(me); //check whether we can initialize already
}

//simple helper struct to bundle all the information that's required to iterate over a hyper rectangle of array indices
//POD object, it does not own or manage any of the referenced memory
typedef struct IndexIterator_t {
  int64_t dimCount;

  //Arrays of length dimCount. , `index` holds the current coordinates.
  int64_t *start, *stop; //the range to iterate over
  int64_t *size;  //the total size of the respective dimension (used to generate the linear index)
  int64_t *index; //the current coordinates that the iterator points to
} IndexIterator_t;

//returns the total number of fragment indices that are relevant for the given bounds
static int64_t getIndexRanges(struct esdmI_regularFragments_t* me, esdmI_hypercube_t* bounds, int64_t* startIndexArray, int64_t* stopIndexArray) {
  //extract the bounds info
  const int64_t dimCount = me->dimCount;
  eassert(esdmI_hypercube_dimensions(bounds) == dimCount);
  int64_t boundsOffset[dimCount], boundsSize[dimCount];
  esdmI_hypercube_getOffsetAndSize(bounds, boundsOffset, boundsSize);

  int64_t indexCount = 1;
  for(int64_t i = 0; i < dimCount; i++) {
    startIndexArray[i] = (boundsOffset[i] - me->fragmentOffset[i])/me->fragmentSize[i];
    stopIndexArray[i] = (boundsOffset[i] + boundsSize[i] - 1 - me->fragmentOffset[i])/me->fragmentSize[i] + 1;  //stopIndexArray is exclusive

    //The bounds may contain elements that are not covered by our dataset, and thus do not have a backing fragment.
    //Bound the indices to the valid range.
    if(startIndexArray[i] < 0) startIndexArray = 0;
    if(stopIndexArray[i] > me->fragmentCount[i]) stopIndexArray[i] = me->fragmentCount[i];

    if(startIndexArray[i] >= stopIndexArray[i]) return 0; //return error if the bounds don't intersect with the dataset
    indexCount *= stopIndexArray[i] - startIndexArray[i]; //the actual count of bins we have to return for this dimension
  }
  return indexCount;
}

static int64_t linearIndex(int64_t dimCount, int64_t* sizeArray, int64_t* indexArray) {
  int64_t result = 0;
  for(int64_t i = 0; i < dimCount; i++) result = result * sizeArray[i] + indexArray[i];
  return result;
}

//this check relies on the behavior of advanceIndex(), it is not a general validity check
static bool indexIsValid(IndexIterator_t* iter, int64_t* out_linearIndex) {
  bool result = iter->index[0] < iter->stop[0];
  *out_linearIndex = result ? linearIndex(iter->dimCount, iter->size, iter->index) : 0;
  return result;
}

static void advanceIndex(IndexIterator_t* iter) {
  //initial increment
  iter->index[iter->dimCount - 1]++;

  //Carry the increment to the larger dimensions as required.
  //
  //This loop iterates only down to 1 to ensure that iter->index[0] == iter->stop[0] after the last iteration,
  //which will be the signal for indexIsValid() to return false.
  for(int64_t i = iter->dimCount; i-- > 1 && iter->index[i] >= iter->stop[i]; ) {
    iter->index[i] = iter->start[i]; //reset this dimension...
    iter->index[i-1]++; //... and increment the next greater dimension
  }
}

static esdmI_hypercube_t* makeBinBounds(struct esdmI_regularFragments_t* me, int64_t* indexArray) {
  int64_t offset[me->dimCount];
  for(int64_t i = 0; i < me->dimCount; i++) {
    offset[i] = me->fragmentOffset[i] + me->fragmentSize[i]*indexArray[i];
  }
  return esdmI_hypercube_make(me->dimCount, offset, me->fragmentSize);
}

static void add(void* meArg, esdm_fragment_t* fragment) {
  struct esdmI_regularFragments_t* me = meArg;
  ensureInitialization(me);

  //determine the range of bins we need for each dimension
  const int64_t dimCount = me->dimCount;
  int64_t startIndex[dimCount], stopIndex[dimCount];
  esdmI_hypercube_t* bounds;
  esdmI_dataspace_getExtends(fragment->dataspace, &bounds);
  int64_t binCount = getIndexRanges(me, bounds, startIndex, stopIndex);
  static int dummy = 0;

  if(binCount) {
    //handle the special case that the given fragment covers exactly one entire bin (which is the case when we are loading a dataset from disk)
    bool isPerfectFit = false;
    if(binCount == 1) {
      esdmI_hypercube_t* binBounds = makeBinBounds(me, startIndex);
      isPerfectFit = esdmI_hypercube_equal(binBounds, bounds);
      esdmI_hypercube_destroy(binBounds);
    }

    if(isPerfectFit) {
      //just replace the possibly existing fragment with the given one
      int64_t fragmentIndex = linearIndex(dimCount, me->fragmentCount, startIndex);
      esdm_fragment_t* bin = me->fragments[fragmentIndex];
      if(bin) {
        eassert(bin->buf);
        //we cannot just replace the possibly existing fragment with the given one, because we do not control its memory buffer
        if(esdm_dataspace_copy_data(fragment->dataspace, fragment->buf, bin->dataspace, bin->buf) != ESDM_SUCCESS) ESDM_ERROR("failed to copy data");
      } else if(fragment->buf == &dummy) {
        //this is a recursive call, the fragment was created without a buffer, and we can take possession of it
        me->fragments[fragmentIndex] = fragment;
        fragment = NULL;  //don't destroy this fragment
      } else if(fragment->status == ESDM_DATA_NOT_LOADED) {
        //the fragment has been created from metadata to connect us to the data stored on disk, take possession of the fragment
        me->fragments[fragmentIndex] = fragment;
        fragment = NULL;  //don't destroy this fragment
      } else {
        //this is not a recursive call, delegate to the normal copying code
        isPerfectFit = false;
      }
    }
    if(!isPerfectFit) {
      //iterate over the touched bins updating/creating the fragments as needed
      int64_t index[dimCount];
      memcpy(index, startIndex, sizeof(index));
      IndexIterator_t iter = {
        .dimCount = dimCount,
        .start = startIndex,
        .stop = stopIndex,
        .size = me->fragmentCount,
        .index = index
      };
      for(int64_t fragmentIndex; indexIsValid(&iter, &fragmentIndex); advanceIndex(&iter)) {
        esdm_fragment_t** bin = &me->fragments[fragmentIndex];
        esdmI_hypercube_t* binExtends = makeBinBounds(me, index);

        //ensure that we have a valid bin fragment and load its data from disk if necessary
        if(*bin) {
          if((*bin)->status == ESDM_DATA_NOT_LOADED) {
            if(!esdmI_hypercube_contains(bounds, binExtends)) {
              //the given data does not fully cover the bin, so we need to load the bin's data into memory for a read-modify-write cycle
              if(esdm_fragment_retrieve(*bin) != ESDM_SUCCESS) ESDM_ERROR("failed to retrieve fragment");
            }
          } else if((*bin)->status == ESDM_DATA_DELETED) {
              ESDM_ERROR("cannot update a deleted fragment");
          } //we already have the data in memory in the cases ESDM_DATA_DIRTY and ESDM_DATA_PERSISTENT, so nothing to be done
        } else {
          int64_t binOffset[dimCount], binSize[dimCount];
          esdmI_hypercube_getOffsetAndSize(binExtends, binOffset, binSize);

          esdm_dataspace_t* subspace;
          if(esdm_dataspace_subspace(me->super.parent->dataspace, dimCount, binSize, binOffset, &subspace) != ESDM_SUCCESS) ESDM_ERROR("failed to create subspace");
          //create the fragment with a dummy buffer pointer to signal the recursive call of this method that it can take possession of the fragment
          if(esdmI_fragment_create(fragment->dataset, subspace, &dummy, fragment->backend, bin) != ESDM_SUCCESS) ESDM_ERROR("failed to create fragment");
          (*bin)->buf = malloc(esdm_dataspace_size((*bin)->dataspace)); //the dummy pointer has served its purpose

          //initialize the fragment's data with the fill value if necessary
          if(!esdmI_hypercube_contains(bounds, binExtends)) {
            esdm_type_t type = esdm_dataspace_get_type((*bin)->dataspace);
            char fillValue[esdm_sizeof(type)];
            if(esdm_dataset_get_fill_value((*bin)->dataset, fillValue) == ESDM_SUCCESS) {
              if(esdm_dataspace_fill((*bin)->dataspace, (*bin)->buf, fillValue) != ESDM_SUCCESS) ESDM_ERROR("failed to initialize data with fill value");
            } else {
              memset((*bin)->buf, 0, esdm_dataspace_size((*bin)->dataspace)); //no fill value set, initialize the fragment to zeros
            }
          }
        }
        //make sure that we actually have a buffer to write to
        //(this triggers when the bin is not loaded and we didn't load it because it's entirely covered by the fragment)
        if(!(*bin)->buf) (*bin)->buf = malloc(esdm_dataspace_size((*bin)->dataspace));

        //copy the data into the bin fragment
        if(esdm_dataspace_copy_data(fragment->dataspace, fragment->buf, (*bin)->dataspace, (*bin)->buf) != ESDM_SUCCESS) ESDM_ERROR("failed to copy data");
        (*bin)->status = ESDM_DATA_DIRTY;
        esdmI_hypercube_destroy(binExtends);
      }
    }
  }

  if(fragment) {
    fragment->status = ESDM_DATA_PERSISTENT;  //we have taken care of persisting all its data
    esdm_fragment_destroy(fragment); //cleanup the fragment if we have copied its data out
  }
  esdmI_hypercube_destroy(bounds);
}

static esdm_fragment_t** list(void* meArg, int64_t* out_fragmentCount) {
  struct esdmI_regularFragments_t* me = meArg;
  ensureInitialization(me);

  eassert(out_fragmentCount && "How can you know how many fragments there are, if you don't let me tell you?");

  *out_fragmentCount = me->totalFragmentCount;
  return me->fragments;
}

static esdm_fragment_t** makeSetCoveringRegion(void* meArg, esdmI_hypercube_t* bounds, int64_t* out_fragmentCount) {
  struct esdmI_regularFragments_t* me = meArg;
  ensureInitialization(me);
  *out_fragmentCount = 0; //for an early error return

  //determine the range of bins we need for each dimension, and compute the max number of fragments we might have to return
  const int64_t dimCount = me->dimCount;
  int64_t startIndex[dimCount], stopIndex[dimCount];
  int64_t maxFragmentCount = getIndexRanges(me, bounds, startIndex, stopIndex);

  //create the set of fragments
  esdm_fragment_t** set = malloc(maxFragmentCount*sizeof(*set));
  int64_t index[dimCount];
  memcpy(index, startIndex, sizeof(index));
  IndexIterator_t iter = {
    .dimCount = dimCount,
    .start = startIndex,
    .stop = stopIndex,
    .size = me->fragmentCount,
    .index = index
  };
  int64_t fragmentCount = 0;  //counts the fragments that we actually return
  for(int64_t fragmentIndex; indexIsValid(&iter, &fragmentIndex); advanceIndex(&iter)) {
    if(me->fragments[fragmentIndex]) {  //add the fragment to the set if it exists
      eassert(fragmentCount < maxFragmentCount);
      set[(fragmentCount)++] = me->fragments[fragmentIndex];
    }
  }

  *out_fragmentCount = fragmentCount;
  return realloc(set, fragmentCount*sizeof(*set));
}

static void metadata_create(void* meArg, smd_string_stream_t* s) {
  struct esdmI_regularFragments_t* me = meArg;
  ensureInitialization(me);

  smd_string_stream_printf(s, "[");
  bool needComma = false;
  for(int i=0; i < me->totalFragmentCount; i++){
    if(me->fragments[i]) {
      if(needComma) smd_string_stream_printf(s, ",\n");
      esdm_fragment_metadata_create(me->fragments[i], s);
      needComma = true;
    }
  }
  smd_string_stream_printf(s, "]");
}

static esdm_status destruct(void* meArg) {
  struct esdmI_regularFragments_t* me = meArg;
  if(me->dimCount < 0) return ESDM_SUCCESS; //fast return if the object was never initialized

  esdm_status result = ESDM_SUCCESS;
  for(int i = me->totalFragmentCount; i--; ) {
    esdm_fragment_t* curFragment = me->fragments[i];
    if(curFragment) {
      //we own the fragments' buffer, so we need to get rid of it
      free(curFragment->buf);
      curFragment->buf = NULL;
      curFragment->status = ESDM_DATA_NOT_LOADED;

      esdm_status ret = esdm_fragment_destroy(curFragment);
      if(ret != ESDM_SUCCESS) result = ret;
    }
  }
  free(me->fragmentSize);
  free(me->fragmentOffset);
  free(me->fragmentCount);
  free(me->fragments);
  return result;
}

static struct esdm_fragments_vtable_t gVtable = {
  .add = add,
  .list = list,
  .makeSetCoveringRegion = makeSetCoveringRegion,
  .metadata_create = metadata_create,
  .destruct = destruct
};
