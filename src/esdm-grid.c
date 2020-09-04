#include <esdm-grid.h>

#include <esdm-debug.h>
#include <esdm-internal.h>

typedef struct esdm_gridEntry_t {
  //At least one of these two pointers must always be NULL.
  esdm_grid_t* subgrid;
  esdm_fragment_t* fragment;
} esdm_gridEntry_t;

typedef struct esdm_axis_t {
  int64_t intervals;
  int64_t outerBounds[2];
  int64_t* allBounds;	//array of intervals+1 entries, allBounds[0] == outerBounds[0] and allBounds[intervals] == outerBounds[1], as long as intervals == 1, this points to outerBounds[0]
} esdm_axis_t;

struct esdm_grid_t {
  int64_t dimCount;
  esdm_dataset_t* dataset;
  esdm_grid_t* parent;
  char* id;

  int64_t emptyCells;	//will be set by esdm_grid_ensureGrid() when the grid is allocated
  esdm_gridEntry_t* grid;

  esdm_axis_t axes[];	//dimCount elements
};

struct esdm_gridIterator_t {
  esdm_gridIterator_t* parent;
  esdm_grid_t* grid;
  int64_t lastIndex, cellCount; //the cell count is local to the grid, subgrid iterators have their own cellCount
};

////////////////////////////////////////////////////////////////////////////////////////////////////

esdm_status esdm_grid_create_internal(esdm_dataset_t* dataset, esdm_grid_t* parent, int64_t dimCount, int64_t* offset, int64_t* size, esdm_grid_t** out_grid) {
  eassert(dataset);
  eassert(dataset->dataspace);
  eassert(dataset->dataspace->dims == dimCount);
  eassert(dimCount > 0);
  eassert(size);
  eassert(out_grid);

  esdm_grid_t* result = ea_checked_malloc(sizeof*result + dimCount*sizeof*result->axes);
  *result = (esdm_grid_t){
    .dimCount = dimCount,
    .dataset = dataset,
    .parent = parent,
    .id = NULL,
    .emptyCells = 0,
    .grid = NULL
  };

  for(int64_t i = 0; i < dimCount; i++) {
    result->axes[i] = (esdm_axis_t){
      .intervals = 1,
      .outerBounds[0] = offset ? offset[i] : 0,
      .outerBounds[1] = (offset ? offset[i] : 0) + size[i],
      .allBounds = result->axes[i].outerBounds
    };
  };

  //only top-level grids are registered with their datasets, subgrids are referenced by their parent grids, only
  if(!parent) esdmI_dataset_registerGrid(dataset, result);

  *out_grid = result;
  return ESDM_SUCCESS;
}

esdm_status esdm_grid_create(esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size, esdm_grid_t** out_grid) {
  return esdm_grid_create_internal(dataset, NULL, dimCount, offset, size, out_grid);
}

esdm_status esdm_grid_createSimple(esdm_dataset_t* dataset, int64_t dimCount, int64_t* size, esdm_grid_t** out_grid) {
  return esdm_grid_create_internal(dataset, NULL, dimCount, NULL, size, out_grid);
}

static int64_t esdmI_grid_cellCount(const esdm_grid_t* grid) {
  int64_t entryCount = 1;
  for(int64_t i = 0; i < grid->dimCount; i++) {
    entryCount *= grid->axes[i].intervals;
  }
  return entryCount;
}

static void esdm_grid_ensureGrid(esdm_grid_t* grid) {
  if(grid->grid) return;

  grid->emptyCells = esdmI_grid_cellCount(grid);
  grid->grid = ea_checked_calloc(grid->emptyCells, sizeof*grid->grid);
}

//returns -1 on error
static int64_t esdm_grid_linearIndex(esdm_grid_t* grid, int64_t* index) {
  int64_t result = 0;
  for(int64_t i = 0; i < grid->dimCount; i++) {
    if(index[i] < 0 || index[i] >= grid->axes[i].intervals) return -1;
    result = result*grid->axes[i].intervals + index[i];
  }
  return result;
}

static void esdm_grid_indexCoordinates(esdm_grid_t* grid, int64_t linearIndex, int64_t* out_index) {
  for(int64_t dim = grid->dimCount; dim--; ) {
    const int64_t intervals = grid->axes[dim].intervals;
    out_index[dim] = linearIndex%intervals;
    linearIndex /= intervals;
  }
  eassert(!linearIndex);
}

esdm_status esdm_grid_createSubgrid(esdm_grid_t* parent, int64_t* index, esdm_grid_t** out_child) {
  eassert(parent);
  eassert(index);
  eassert(out_child);
  if(parent->id) return ESDM_INVALID_STATE_ERROR; //the grid is in fixed structure state, and adding a subgrid is changing the structure

  *out_child = NULL;
  int64_t linearIndex = esdm_grid_linearIndex(parent, index);
  if(linearIndex < 0) return ESDM_INVALID_ARGUMENT_ERROR;

  esdm_grid_ensureGrid(parent);
  if(parent->grid[linearIndex].subgrid || parent->grid[linearIndex].fragment) return ESDM_INVALID_STATE_ERROR;

  int64_t offset[parent->dimCount], size[parent->dimCount];
  esdm_status result = esdm_grid_cellSize(parent, index, offset, size);
  if(result != ESDM_SUCCESS) return result;

  esdm_grid_t* subgrid;
  result = esdm_grid_create_internal(parent->dataset, parent, parent->dimCount, offset, size, &subgrid);
  if(result == ESDM_SUCCESS) *out_child = parent->grid[linearIndex].subgrid = subgrid;
  return result;
}

esdm_status esdm_grid_subdivideFixed(esdm_grid_t* grid, int64_t dim, int64_t size, bool allowIncomplete) {
  eassert(grid);

  if(dim < 0 || dim >= grid->dimCount) return ESDM_INVALID_ARGUMENT_ERROR;
  if(grid->grid) return ESDM_INVALID_STATE_ERROR;

  esdm_axis_t* axis = &grid->axes[dim];
  int64_t axisSize = axis->outerBounds[1] - axis->outerBounds[0];
  if(axisSize < size) return ESDM_INVALID_ARGUMENT_ERROR;
  if(!allowIncomplete && axisSize%size) return ESDM_INVALID_ARGUMENT_ERROR;

  if(axis->intervals != 1) free(axis->allBounds);
  axis->intervals = (axisSize + size - 1)/size;
  axis->allBounds = axis->intervals == 1
                  ? axis->outerBounds
                  : ea_checked_malloc((axis->intervals + 1)*sizeof*axis->allBounds);
  for(int64_t i = 0; i <= axis->intervals; i++) {
    axis->allBounds[i] = axis->outerBounds[0] + (size*i < axisSize ? size*i : axisSize);
  }

  return ESDM_SUCCESS;
}

esdm_status esdm_grid_subdivideFlexible(esdm_grid_t* grid, int64_t dim, int64_t count) {
  eassert(grid);

  if(dim < 0 || dim >= grid->dimCount) return ESDM_INVALID_ARGUMENT_ERROR;
  if(grid->grid) return ESDM_INVALID_STATE_ERROR;

  esdm_axis_t* axis = &grid->axes[dim];
  int64_t axisSize = axis->outerBounds[1] - axis->outerBounds[0];
  if(axisSize < count) return ESDM_INVALID_ARGUMENT_ERROR;

  if(axis->intervals != 1) free(axis->allBounds);
  axis->intervals = count;
  axis->allBounds = count == 1
                  ? axis->outerBounds
                  : ea_checked_malloc((count + 1)*sizeof*axis->allBounds);
  for(int64_t i = 0; i <= count; i++) {
    axis->allBounds[i] = axis->outerBounds[0] + axisSize*i/count;
  }

  return ESDM_SUCCESS;
}

esdm_status esdm_grid_subdivide(esdm_grid_t* grid, int64_t dim, int64_t count, int64_t* bounds) {
  eassert(grid);
  eassert(count > 0);
  eassert(bounds);

  if(dim < 0 || dim >= grid->dimCount) return ESDM_INVALID_ARGUMENT_ERROR;
  if(grid->grid) return ESDM_INVALID_STATE_ERROR;

  //sanity check of the given bounds
  esdm_axis_t* axis = &grid->axes[dim];
  if(bounds[0] != axis->outerBounds[0]) return ESDM_INVALID_ARGUMENT_ERROR;
  if(bounds[count] != axis->outerBounds[1]) return ESDM_INVALID_ARGUMENT_ERROR;
  for(int64_t i = 0; i < count; i++) {
    if(bounds[i] >= bounds[i+1]) return ESDM_INVALID_ARGUMENT_ERROR;
  }

  //input is ok, commit the change
  if(axis->intervals != 1) free(axis->allBounds);
  axis->intervals = count;
  axis->allBounds = count == 1
                  ? axis->outerBounds
                  : ea_memdup(bounds, (count + 1)*sizeof*bounds);
  _Static_assert(sizeof*bounds == sizeof*axis->allBounds, "types must match");

  return ESDM_SUCCESS;
}

esdm_status esdm_grid_cellSize(const esdm_grid_t* grid, int64_t* cellIndex, int64_t* out_offset, int64_t* out_size) {
  eassert(grid);
  eassert(cellIndex);
  eassert(out_offset);
  eassert(out_size);

  for(int64_t dim = 0; dim < grid->dimCount; dim++) {
    const esdm_axis_t* axis = &grid->axes[dim];
    const int64_t index = cellIndex[dim];
    if(index < 0 || index >= axis->intervals) return ESDM_INVALID_ARGUMENT_ERROR;
    out_offset[dim] = axis->allBounds[index];
    out_size[dim] = axis->allBounds[index + 1] - axis->allBounds[index];
  }
  return ESDM_SUCCESS;
}

esdm_status esdm_grid_cellExtends(const esdm_grid_t* grid, int64_t* cellIndex, esdmI_hypercube_t** out_extends) {
  eassert(grid);
  eassert(out_extends);

  int64_t offset[grid->dimCount], size[grid->dimCount];
  esdm_status result = esdm_grid_cellSize(grid, cellIndex, offset, size);
  if(result != ESDM_SUCCESS) return result;
  *out_extends = esdmI_hypercube_make(grid->dimCount, offset, size);
  return ESDM_SUCCESS;
}

esdm_status esdm_grid_getBound(const esdm_grid_t* grid, int64_t dim, int64_t index, int64_t* out_bound) {
  eassert(grid);

  if(dim < 0 || dim >= grid->dimCount) return ESDM_INVALID_ARGUMENT_ERROR;
  const esdm_axis_t* axis = &grid->axes[dim];
  if(index < 0 || index > axis->intervals) return ESDM_INVALID_ARGUMENT_ERROR;
  *out_bound = axis->allBounds[index];
  return ESDM_SUCCESS;
}

static void esdmI_grid_registerCompletedCell(esdm_grid_t* grid) {
  eassert(grid->emptyCells > 0);
  if(grid->emptyCells-- == 1) {
    if(grid->parent) {
      esdmI_grid_registerCompletedCell(grid->parent);
    } else {
      esdmI_dataset_registerGridCompletion(grid->dataset, grid);
    }
  }
}

//Optimized binary search for the interval that contains the given location.
//The result is either -1 if the location is out of bounds, or in the inclusive range [0, intervals-1].
//The value `intervals` is never returned because `outerBounds[1]` only marks the end of the last interval but is not part of any interval itself.
static int64_t esdmI_axis_findInterval(esdm_axis_t* axis, int64_t location) {
  if(location < axis->outerBounds[0] || location >= axis->outerBounds[1]) return -1;

  int64_t startIndex = 0, endIndex = axis->intervals;
  while(startIndex + 1 < endIndex) {
    //find an index with a bound that is likely close to the requested location
    int64_t midIndex = startIndex + (endIndex - startIndex)*(location - axis->allBounds[startIndex])/(axis->allBounds[endIndex] - axis->allBounds[startIndex]);
    eassert(startIndex <= midIndex && midIndex <= endIndex);

    //ensure progress
    //because endIndex is at least startIndex+2, this ensures that the midIndex is neither the startIndex nor the endIndex
    if(midIndex == startIndex) {
      midIndex++;
    } else if(midIndex == endIndex) {
      midIndex--;
    }
    eassert(startIndex < midIndex && midIndex < endIndex);

    //select subinterval
    if(axis->allBounds[midIndex] > location) {
      endIndex = midIndex;
    } else {
      startIndex = midIndex;
    }
  }
  eassert(startIndex + 1 == endIndex);

  return startIndex;
}

//Translate logical coordinates into the index of the enclosing cell.
//Does not recurs into subgrids.
//The given coordinates must be within the grid domain.
static void esdmI_grid_logicalToCellIndex(esdm_grid_t* grid, int64_t* logicalCoords, int64_t* out_cellIndex) {
  for(int64_t i = grid->dimCount; i--; ) out_cellIndex[i] = esdmI_axis_findInterval(&grid->axes[i], logicalCoords[i]);
}

//Match the given interval against the intervals on the axis.
//If the `start`/`end` spans more than a single axis interval, returns -1,
//otherwise the index of the axis interval containing the given interval is returned.
//If the axis interval is longer than `end-start`, `*out_isShort` is set to true,
//otherwise, `*out_isShort` is left untouched.
static int64_t esdmI_axis_matchInterval(esdm_axis_t* axis, int64_t start, int64_t end, bool* out_isShort) {
  if(start >= end) return -1;
  if(start < axis->outerBounds[0] || end > axis->outerBounds[1]) return -1;

  int64_t intervalIndex = esdmI_axis_findInterval(axis, start);
  eassert(intervalIndex < axis->intervals);
  if(intervalIndex < 0) return -1;

  if(end > axis->allBounds[intervalIndex + 1]) return -1;
  if(start > axis->allBounds[intervalIndex] || end < axis->allBounds[intervalIndex + 1]) *out_isShort = true;
  return intervalIndex;
}

//This will throw an error if the dataspace is bigger than the grid cell.
//In the opposite case (dataspace too small), no error is produced,
//but the condition is signalled back to the caller via the out_spaceTooSmall argument.
static esdm_status esdmI_grid_findCell(esdm_grid_t* grid, esdm_dataspace_t* space, int64_t* out_index, bool* out_spaceTooSmall) {
  *out_spaceTooSmall = false;
  for(int64_t dim = 0; dim < grid->dimCount; dim++) {
    int64_t curIndex = esdmI_axis_matchInterval(&grid->axes[dim], space->offset[dim], space->offset[dim] + space->size[dim], out_spaceTooSmall);
    if(curIndex < 0) return ESDM_ERROR;
    out_index[dim] = curIndex;
  }
  return ESDM_SUCCESS;
}

//This is the recursive version of `esdm_grid_findCell()`.
//Returns a grid cell that fits the dataspace exactly, and that does not contain a subgrid.
//The `*inout_grid` argument will be updated to the subgrid that actually contains the cell.
static esdm_status esdm_grid_findCellInHierarchy(esdm_grid_t** inout_grid, esdm_dataspace_t* space, esdm_gridEntry_t** out_cell) {
  //Find the grid cell that matches the memspace.
  //This may replace the grid with one of its transitive subgrids.
  esdm_grid_t* grid = *inout_grid;
  esdm_gridEntry_t* cell = NULL;
  while(true) {
    //Determine the indices of the grid cell we are supposed to manipulate.
    int64_t index[grid->dimCount];
    bool spaceIsTooSmall;
    esdm_status result = esdmI_grid_findCell(grid, space, index, &spaceIsTooSmall);
    if(result != ESDM_SUCCESS) return ESDM_INVALID_ARGUMENT_ERROR; //if the space is bigger than a cell
    int64_t linearIndex = esdm_grid_linearIndex(grid, index);
    eassert(linearIndex >= 0 && "esdmI_grid_findCell() must return a valid index");
    if(!grid->grid && spaceIsTooSmall) return ESDM_INVALID_ARGUMENT_ERROR; //no grid allocated -> no subgrids present -> space must match a cell exactly

    //Check the grid cell itself and descend the grid tree if necessary.
    esdm_grid_ensureGrid(grid);
    cell = &grid->grid[linearIndex];
    if(!cell->subgrid) {
      if(spaceIsTooSmall) return ESDM_INVALID_ARGUMENT_ERROR;  //no subgrid -> space must match the cell exactly
      break;  //got the cell that fits the space exactly
    }
    grid = cell->subgrid;
  }
  *inout_grid = grid;
  *out_cell = cell;
  return ESDM_SUCCESS;
}

esdm_status esdm_write_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, void* buffer) {
  eassert(grid);
  eassert(memspace);
  eassert(buffer);

  if(grid->dimCount != memspace->dims) return ESDM_INVALID_ARGUMENT_ERROR;

  //Find the grid cell that matches the memspace.
  //This may replace the grid with one of its transitive subgrids.
  esdm_gridEntry_t* cell;
  esdm_status result = esdm_grid_findCellInHierarchy(&grid, memspace, &cell);
  if(result != ESDM_SUCCESS) return result;
  if(cell->fragment) return ESDM_SUCCESS; //we already have data for this grid cell -> nothing to be done

  //Do the actual writing.
  esdm_dataspace_t* fragmentSpace;
  result = esdm_dataspace_copy(memspace, &fragmentSpace);
  if(result == ESDM_SUCCESS) result = esdmI_scheduler_writeSingleFragmentBlocking(esdmI_esdm(), grid->dataset, buffer, fragmentSpace, false, &cell->fragment);
  if(result == ESDM_SUCCESS) {
    esdmI_dataset_register_fragment(grid->dataset, cell->fragment, true);
    esdmI_grid_registerCompletedCell(grid);
  }
  return result;
}

esdm_status esdm_read_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, void* buffer) {
  eassert(grid);
  eassert(memspace);
  eassert(buffer);

  if(grid->dimCount != memspace->dims) return ESDM_INVALID_ARGUMENT_ERROR;

  //Find the grid cell that matches the memspace.
  //This may replace the grid with one of its transitive subgrids.
  esdm_gridEntry_t* cell;
  esdm_status result = esdm_grid_findCellInHierarchy(&grid, memspace, &cell);
  if(result != ESDM_SUCCESS) return result;
  if(!cell->fragment) return ESDM_INCOMPLETE_DATA;

  //Do the actual reading.
  result = esdmI_scheduler_readSingleFragmentBlocking(esdmI_esdm(), grid->dataset, buffer, memspace, cell->fragment);
  return result;
}

esdm_status esdm_dataset_grids(esdm_dataset_t* dataset, int64_t* out_count, esdm_grid_t*** out_grids) {
  eassert(dataset);
  eassert(out_count);

  *out_count = dataset->gridCount;
  if(out_grids) *out_grids = ea_memdup(dataset->grids, dataset->gridCount*sizeof*dataset->grids);

  return ESDM_SUCCESS;
}

static esdm_gridIterator_t* esdm_gridIterator_create_internal(esdm_grid_t* grid, esdm_gridIterator_t* parent) {
  esdm_gridIterator_t* result = ea_checked_malloc(sizeof*result);
  *result = (esdm_gridIterator_t){
    .parent = parent,
    .grid = grid,
    .lastIndex = -1,
    .cellCount = esdmI_grid_cellCount(grid)
  };
  return result;
}

esdm_status esdm_gridIterator_create(esdm_grid_t* grid, esdm_gridIterator_t** out_iterator) {
  eassert(grid);
  eassert(out_iterator);

  if(grid->emptyCells) return ESDM_INVALID_STATE_ERROR;
  *out_iterator = esdm_gridIterator_create_internal(grid, NULL);
  return ESDM_SUCCESS;
}

static void esdm_gridIterator_advance(esdm_gridIterator_t** inout_iterator, int64_t increment) {
  esdm_gridIterator_t* iterator = *inout_iterator;
  while(increment) {
    if(++(iterator->lastIndex) >= iterator->cellCount) {
      //we've reached the end of this grid, ascend one iterator level
      esdm_gridIterator_t* temp = iterator;
      iterator = iterator->parent;
      free(temp);
      if(!iterator) break;

      continue; //need to advance the parent iterator instead
    }

    esdm_gridEntry_t* entry = &iterator->grid->grid[iterator->lastIndex];
    if(entry->subgrid) {
      //we've stumbled across a subgrid, descend into a subiterator
      iterator = esdm_gridIterator_create_internal(entry->subgrid, iterator);
      continue; //need to advance the subiterator instead
    }

    increment--;  //all ascending/descending done, we've reached the next grid cell
  }
  *inout_iterator = iterator;
}

esdm_status esdm_gridIterator_next(esdm_gridIterator_t** inout_iterator, int64_t increment, esdm_dataspace_t** out_dataspace) {
  eassert(inout_iterator);
  eassert(*inout_iterator);

  esdm_gridIterator_advance(inout_iterator, increment);
  if(!*inout_iterator) {
    *out_dataspace = NULL;
    return ESDM_SUCCESS;
  }

  esdm_gridIterator_t* const iterator = *inout_iterator;
  const int64_t dimCount = iterator->grid->dimCount;
  int64_t cellIndex[dimCount], offset[dimCount], size[dimCount];
  esdm_grid_indexCoordinates(iterator->grid, iterator->lastIndex, cellIndex);
  esdm_status result = esdm_grid_cellSize(iterator->grid, cellIndex, offset, size);
  eassert(result == ESDM_SUCCESS);
  return esdm_dataspace_subspace(iterator->grid->dataset->dataspace, dimCount, size, offset, out_dataspace);
}

void esdm_gridIterator_destroy(esdm_gridIterator_t* iterator) {
  while(iterator) {
    esdm_gridIterator_t* parent = iterator->parent;
    free(iterator);
    iterator = parent;
  }
}

//TODO: Move this into esdm-datatypes.
void esdmI_dataset_registerGrid(esdm_dataset_t* dataset, esdm_grid_t* grid) {
  if(dataset->gridCount + dataset->incompleteGridCount == dataset->gridSlotCount) {
    dataset->grids = ea_checked_realloc(dataset->grids, (dataset->gridSlotCount *= 2)*sizeof*dataset->grids);
  }
  eassert(dataset->gridCount + dataset->incompleteGridCount < dataset->gridSlotCount);

  dataset->grids[dataset->gridCount + dataset->incompleteGridCount++] = grid;
}

//TODO: Move this into esdm-datatypes.
void esdmI_dataset_registerGridCompletion(esdm_dataset_t* dataset, esdm_grid_t* grid) {
  eassert(dataset->incompleteGridCount && "there must be an incomplete grid for esdmI_dataset_registerGridCompletion() to be called");

  int64_t index = dataset->gridCount;
  while(index < dataset->gridCount + dataset->incompleteGridCount) {
    if(dataset->grids[index] == grid) break;
    index++;
  }
  eassert(index < dataset->gridCount + dataset->incompleteGridCount && "registerGridCompletion() must be called with an existing incomplete grid");

  esdm_grid_t* temp = dataset->grids[index];
  dataset->grids[index] = dataset->grids[dataset->gridCount];
  dataset->grids[dataset->gridCount] = temp;
  dataset->gridCount++, dataset->incompleteGridCount--;
}

#define min(x, y) (x < y ? x : y)
#define max(x, y) (x > y ? x : y)

int64_t esdmI_grid_coverRegionSize(const esdm_grid_t* grid, const esdmI_hypercube_t* region) {
  int64_t dimCount = esdmI_hypercube_dimensions(region);
  eassert(dimCount == grid->dimCount);

  int64_t offset[dimCount], size[dimCount];
  esdmI_hypercube_getOffsetAndSize(region, offset, size);
  int64_t result = 1;
  for(int64_t dim = 0; dim < dimCount; dim++) {
    const esdm_axis_t* axis = &grid->axes[dim];
    int64_t start = max(offset[dim], axis->outerBounds[0]);
    int64_t end = min(offset[dim] + size[dim], axis->outerBounds[1]);
    result *= max(0, end - start);
  }

  return result;
}

int64_t esdmI_grid_coverRegionOverhead(esdm_grid_t* grid, esdmI_hypercube_t* region) {
  int64_t dimCount = esdmI_hypercube_dimensions(region);
  eassert(dimCount == grid->dimCount);

  int64_t offset[dimCount], size[dimCount];
  esdmI_hypercube_getOffsetAndSize(region, offset, size);
  int64_t coverRegionSize = 1, fetchedRegionSize = 1;
  for(int64_t dim = 0; dim < dimCount; dim++) {
    //Copy-pasta from esdmI_grid_coverRegionSize() to avoid the two separate passes over the axes that a call to `esdmI_grid_coverRegionSize()` would have caused.
    esdm_axis_t* axis = &grid->axes[dim];
    int64_t start = max(offset[dim], axis->outerBounds[0]);
    int64_t end = min(offset[dim] + size[dim], axis->outerBounds[1]);
    coverRegionSize *= max(0, end - start);

    int64_t startInterval = esdmI_axis_findInterval(axis, start);
    eassert(startInterval >= 0);
    int64_t endInterval = esdmI_axis_findInterval(axis, end - 1); //-1 to get the interval that actually contains the last location within the region
    eassert(endInterval >= 0);
    fetchedRegionSize *= axis->allBounds[endInterval + 1] - axis->allBounds[startInterval];
  }

  return fetchedRegionSize - coverRegionSize;
}

typedef struct esdmI_grid_fragmentsInRegion_state_t {
  int64_t fragmentCount, bufferSize;
  esdm_fragment_t** fragments;
} esdmI_grid_fragmentsInRegion_state_t;

static void esdmI_grid_fragmentsInRegion_addFragment(esdmI_grid_fragmentsInRegion_state_t* state, esdm_fragment_t* fragment) {
  if(state->fragmentCount == state->bufferSize) {
    state->fragments = ea_checked_realloc(state->fragments, (state->bufferSize *= 2)*sizeof*state->fragments);
  }
  eassert(state->fragmentCount < state->bufferSize);

  state->fragments[state->fragmentCount++] = fragment;
}

//Compute the range of cell indices that overlaps with the given region.
//`out_startIndex` and `out_endIndex` are arrays of size grid->dimCount.
static void esdmI_grid_indexRange(esdm_grid_t* grid, esdmI_hypercube_t* region, int64_t* out_startIndex, int64_t* out_endIndex) {
  int64_t dimCount = esdmI_hypercube_dimensions(region);
  eassert(grid->dimCount == dimCount);

  int64_t start[dimCount], size[dimCount], end[dimCount];
  esdmI_hypercube_getOffsetAndSize(region, start, size);
  for(int64_t i = 0; i < dimCount; i++) {
    end[i] = start[i] + size[i];

    //start and end may be outside the grid domain. Clip them to the domain.
    //If the entire region happens to be outside the grid domain, this will yield an empty coordinate range.
    int64_t firstBound = grid->axes[i].outerBounds[0], lastBound = grid->axes[i].outerBounds[1];
    start[i] = max(start[i], firstBound);
    start[i] = min(start[i], lastBound);
    end[i] = max(end[i], firstBound);
    end[i] = min(end[i], lastBound);

    end[i]--; //turn it into an inclusive end coordinate, this is important for `esdmI_grid_logicalToCellIndex()` to yield the correct result in all cases
  }

  esdmI_grid_logicalToCellIndex(grid, start, out_startIndex);
  esdmI_grid_logicalToCellIndex(grid, end, out_endIndex);
  for(int64_t i = 0; i < dimCount; i++) out_endIndex[i]++;  //back to an exclusive end index
}

static esdm_status esdmI_grid_fragmentsInRegion_internal(esdm_grid_t* grid, esdmI_hypercube_t* region, esdmI_grid_fragmentsInRegion_state_t* state) {
  int64_t startIndex[grid->dimCount], endIndex[grid->dimCount];
  esdmI_grid_indexRange(grid, region, startIndex, endIndex);
  eassert(memcmp(startIndex, endIndex, sizeof(startIndex)));

  int64_t curIndex[grid->dimCount], linearIndex = esdm_grid_linearIndex(grid, startIndex);
  eassert(linearIndex >= 0);
  memcpy(curIndex, startIndex, sizeof(curIndex));
  while(true) {
    if(grid->grid[linearIndex].fragment) {
      esdmI_grid_fragmentsInRegion_addFragment(state, grid->grid[linearIndex].fragment);
    } else if(grid->grid[linearIndex].subgrid) {
      esdm_status result = esdmI_grid_fragmentsInRegion_internal(grid->grid[linearIndex].subgrid, region, state);
      if(result != ESDM_SUCCESS) return result;
    } else {
      return ESDM_INVALID_STATE_ERROR;
    }

    int64_t increment = 1;
    bool indexIncremented = false;
    for(int64_t dim = grid->dimCount; !indexIncremented && dim--; increment *= grid->axes[dim].intervals) {
      linearIndex += increment;
      if(++curIndex[dim] != endIndex[dim]) {
        indexIncremented = true;
      } else {
        linearIndex -= (curIndex[dim] - startIndex[dim])*increment;
        curIndex[dim] = startIndex[dim];
      }
    }
    if(!indexIncremented) break;  //done if we couldn't increment the index
  }

  return ESDM_SUCCESS;
}

esdm_status esdmI_grid_fragmentsInRegion(esdm_grid_t* grid, esdmI_hypercube_t* region, int64_t* out_fragmentCount, esdm_fragment_t*** out_fragments) {
  eassert(grid->grid);

  esdmI_grid_fragmentsInRegion_state_t state = {
    .fragmentCount = 0,
    .bufferSize = 8,
    .fragments = ea_checked_malloc(8*sizeof*state.fragments)
  };
  esdm_status result = esdmI_grid_fragmentsInRegion_internal(grid, region, &state);

  if(result == ESDM_SUCCESS) {
    *out_fragmentCount = state.fragmentCount;
    *out_fragments = state.fragments;
  } else {
    free(state.fragments);
  }

  return result;
}

void esdmI_grid_serialize(smd_string_stream_t* stream, esdm_grid_t* grid) {
  eassert(stream);
  eassert(grid);

  //put the grid into fixed structure state
  esdm_grid_ensureGrid(grid);
  if(!grid->id) grid->id = ea_make_id(23);

  smd_string_stream_printf(stream, "{\"axes\":[");
  for(int64_t dim = 0; dim < grid->dimCount; dim++) {
    smd_string_stream_printf(stream, "%s[", dim ? "," : "");
    const esdm_axis_t* axis = &grid->axes[dim];
    for(int64_t i = 0; i <= axis->intervals; i++) {
      smd_string_stream_printf(stream, "%s%"PRId64, (i ? "," : ""), axis->allBounds[i]);
    }
    smd_string_stream_printf(stream, "]");
  }

  smd_string_stream_printf(stream, "],\"id\":\"%s\",\"grid\":[", grid->id);

  int64_t cellCount = esdmI_grid_cellCount(grid);
  for(int64_t i = 0; i < cellCount; i++) {
    const esdm_gridEntry_t* cell = &grid->grid[i];
    eassert(!cell->subgrid || !cell->fragment);

    if(i) smd_string_stream_printf(stream, ",");
    if(cell->subgrid) {
      smd_string_stream_printf(stream, "{\"grid\":");
      esdmI_grid_serialize(stream, cell->subgrid);
      smd_string_stream_printf(stream, "}");
    } else if(cell->fragment) {
      eassert(cell->fragment->id);
      smd_string_stream_printf(stream, "{\"fragment\":");
      esdm_fragment_metadata_create(cell->fragment, stream);
      smd_string_stream_printf(stream, "}");
    } else {
      smd_string_stream_printf(stream, "{}");
    }
  }
  smd_string_stream_printf(stream, "]}");
}

esdm_status esdmI_grid_createFromJson(json_t* json, esdm_dataset_t* dataset, esdm_grid_t* parent, esdm_grid_t** out_grid) {
  eassert(json);
  eassert(dataset);

  //state that's relevant for error cleanup
  esdm_status result = ESDM_ERROR;
  esdm_grid_t* grid = NULL;
  int64_t constructedAxes = 0;
  int64_t constructedCells = 0;

  //other state that must be forward declared due to the gotos
  int64_t dimCount, cellCount;
  const char* id;
  json_t* axesArray, *cellArray, *idString;

  if(!json_is_object(json)) goto fail;

  axesArray = json_object_get(json, "axes");
  if(!axesArray) goto fail;
  if(!json_is_array(axesArray)) goto fail;
  dimCount = json_array_size(axesArray);

  cellArray = json_object_get(json, "grid");
  if(!cellArray) goto fail;
  if(!json_is_array(cellArray)) goto fail;
  cellCount = json_array_size(cellArray);

  idString = json_object_get(json, "id");
  if(!idString) goto fail;
  if(!json_is_string(idString)) goto fail;
  id = json_string_value(idString);

  grid = ea_checked_malloc(sizeof*grid + dimCount*sizeof*grid->axes);
  *grid = (esdm_grid_t){
    .dimCount = dimCount,
    .parent = parent,
    .dataset = dataset,
    .emptyCells = 0,
    .id = ea_checked_strdup(id),
    .grid = ea_checked_calloc(cellCount, sizeof*grid->grid)
  };
  if(!grid->id) goto fail;

  for(; constructedAxes < dimCount; constructedAxes++) {
    esdm_axis_t* axis = &grid->axes[constructedAxes];
    json_t* boundsArray = json_array_get(axesArray, constructedAxes);
    if(!boundsArray) goto fail;
    if(!json_is_array(boundsArray)) goto fail;
    int64_t boundCount = json_array_size(boundsArray);
    axis->intervals = boundCount - 1;
    if(boundCount < 2) goto fail;
    axis->allBounds = axis->intervals == 1 ? axis->outerBounds : ea_checked_malloc(boundCount*sizeof*axis->allBounds);
    for(int64_t i = 0; i < boundCount; i++) {
      json_t* bound = json_array_get(boundsArray, i);
      if(!bound) goto fail;
      if(!json_is_integer(bound)) goto fail;
      axis->allBounds[i] = json_integer_value(bound);
    }
    axis->outerBounds[0] = axis->allBounds[0];
    axis->outerBounds[1] = axis->allBounds[axis->intervals];
  }
  if(cellCount != esdmI_grid_cellCount(grid)) goto fail;

  for(; constructedCells < cellCount; constructedCells++) {
    esdm_gridEntry_t* cell = &grid->grid[constructedCells];
    json_t* cellObject = json_array_get(cellArray, constructedCells);
    if(!cellObject) goto fail;
    if(!json_is_object(cellObject)) goto fail;

    json_t* gridElement = json_object_get(cellObject, "grid");
    json_t* fragmentElement = json_object_get(cellObject, "fragment");
    if(gridElement && fragmentElement) {
      goto fail;
    } else if(gridElement) {
      esdm_status ret = esdmI_grid_createFromJson(gridElement, dataset, grid, &cell->subgrid);
      if(ret != ESDM_SUCCESS) goto fail;
    } else if(fragmentElement) {
      if(!json_is_object(fragmentElement)) goto fail;
      esdm_status ret = esdmI_create_fragment_from_metadata(dataset, fragmentElement, &cell->fragment);
      if(ret != ESDM_SUCCESS) goto fail;
    } else {
      grid->emptyCells++;
    }
  }

  //construction of the grid object was successful, register the grid with the dataset
  if(!parent) {
    esdmI_dataset_registerGrid(dataset, grid);
    if(!grid->emptyCells) esdmI_dataset_registerGridCompletion(dataset, grid);
  }

  result = ESDM_SUCCESS;
  goto done;

fail:
  while(constructedCells--) {
    esdm_gridEntry_t* cell = &grid->grid[constructedCells];
    if(cell->subgrid) esdmI_grid_destroy(cell->subgrid);
  }

  while(constructedAxes--) {
    esdm_axis_t* axis = &grid->axes[constructedAxes];
    if(axis->intervals != 1) free(axis->allBounds);
  }

  if(grid) {
    free(grid->id);
    free(grid->grid);
    free(grid);
  }
  grid = NULL;

done:
  *out_grid = grid;
  return result;
}

esdm_status esdmI_grid_createFromString(const char* serializedGrid, esdm_dataset_t* dataset, esdm_grid_t** out_grid) {
  json_t* json = load_json(serializedGrid);
  if(!json) return ESDM_ERROR;
  esdm_status result = esdmI_grid_createFromJson(json, dataset, NULL, out_grid);
  json_decref(json);
  return result;
}

esdm_status esdmI_grid_mergeWithJson(esdm_grid_t* grid, json_t* json) {
  eassert(grid);
  eassert(grid->id);
  eassert(grid->grid);
  eassert(json);

  //check that the grid IDs match (i.e. that the serialized grid is actually a copy of this grid)
  if(!json_is_object(json)) return ESDM_INVALID_DATA_ERROR;
  json_t* jsonId = json_object_get(json, "id");
  if(!jsonId || !json_is_string(jsonId)) return ESDM_INVALID_DATA_ERROR;
  const char* id = json_string_value(jsonId);
  if(strcmp(grid->id, id)) return ESDM_INVALID_STATE_ERROR;

  //walk the grid cells and fill in any holes with fragments from the serialized grid
  json_t* jsonGrid = json_object_get(json, "grid");
  if(!jsonGrid || !json_is_array(jsonGrid)) return ESDM_INVALID_DATA_ERROR;
  int64_t cellCount = json_array_size(jsonGrid);
  if(cellCount != esdmI_grid_cellCount(grid)) return ESDM_INVALID_DATA_ERROR;
  for(int64_t i = 0; i < cellCount; i++) {
    esdm_gridEntry_t* cell = &grid->grid[i];
    eassert(!cell->fragment || !cell->subgrid);
    if(cell->fragment) continue;  //no need to deserialize anything if we have already data for this cell
    if(cell->subgrid) {
      if(!cell->subgrid->emptyCells) continue;  //no need to recurs into subgrid if we already have complete data for it
      json_t* jsonCell = json_array_get(jsonGrid, i);
      if(!jsonCell || !json_is_object(jsonCell)) return ESDM_INVALID_DATA_ERROR;
      json_t* jsonSubgrid = json_object_get(jsonCell, "subgrid");
      if(jsonSubgrid) {
        if(!json_is_object(jsonSubgrid)) return ESDM_INVALID_DATA_ERROR;
        esdm_status result = esdmI_grid_mergeWithJson(cell->subgrid, jsonSubgrid);
        if(result != ESDM_SUCCESS) return result;
      }
    } else {
      json_t* jsonCell = json_array_get(jsonGrid, i);
      if(!jsonCell || !json_is_object(jsonCell)) return ESDM_INVALID_DATA_ERROR;
      json_t* jsonFragment = json_object_get(jsonCell, "fragment");
      if(jsonFragment) {
        if(!json_is_object(jsonFragment)) return ESDM_INVALID_DATA_ERROR;
        esdm_status result = esdmI_create_fragment_from_metadata(grid->dataset, jsonFragment, &cell->fragment);
        if(result != ESDM_SUCCESS) return result;
        esdmI_grid_registerCompletedCell(grid);
      }
    }
  }

  return ESDM_SUCCESS;
}

esdm_status esdmI_grid_mergeWithString(esdm_grid_t* grid, const char* serializedGrid) {
  json_t* json = load_json(serializedGrid);
  if(!json) return ESDM_ERROR;
  esdm_status result = esdmI_grid_mergeWithJson(grid, json);
  json_decref(json);
  return result;
}

const char* esdmI_grid_id(esdm_grid_t* grid) {
  return grid->id;
}

bool esdmI_grid_matchesId(const esdm_grid_t* grid, const char* id) {
  if(!grid->id) return false;
  return 0 == strcmp(grid->id, id);
}

void esdmI_grid_destroy(esdm_grid_t* grid) {
  if(grid->grid) {
    for(int64_t i = esdmI_grid_cellCount(grid); i--; ) {
      if(grid->grid[i].subgrid) esdmI_grid_destroy(grid->grid[i].subgrid);
      if(grid->grid[i].fragment) esdm_fragment_destroy(grid->grid[i].fragment);
    }
    free(grid->grid);
  }
  for(int64_t dim = grid->dimCount; dim--; ) {
    if(grid->axes[dim].intervals != 1) free(grid->axes[dim].allBounds);
  }
  free(grid);
}
