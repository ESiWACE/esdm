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

  int64_t emptyCells;	//will be set by esdm_grid_ensureGrid() when the grid is allocated
  esdm_gridEntry_t* grid;

  esdm_axis_t axes[];	//dimCount elements
};

struct esdm_gridIterator_t {
  esdm_gridIterator_t* parent;
  esdm_grid_t* grid;
  int64_t lastIndex, cellCount; //the cell count is local to the grid, subgrid iterators have their own cellCount
};

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

static int64_t esdmI_grid_cellCount(esdm_grid_t* grid) {
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
    result = result*grid->axes[i].intervals + i;
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

  *out_child = NULL;
  int64_t linearIndex = esdm_grid_linearIndex(parent, index);
  if(linearIndex < 0) return ESDM_INVALID_ARGUMENT_ERROR;

  esdm_grid_ensureGrid(parent);
  if(parent->grid[linearIndex].subgrid || parent->grid[linearIndex].fragment) return ESDM_INVALID_STATE_ERROR;

  int64_t offset[parent->dimCount], size[parent->dimCount];
  esdm_status result = esdm_grid_cellSize(parent, index, offset, size);
  if(result != ESDM_SUCCESS) return result;

  result = esdm_grid_create_internal(parent->dataset, parent, parent->dimCount, offset, size, &parent->grid[linearIndex].subgrid);
  if(result == ESDM_SUCCESS) parent->emptyCells--;
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

esdm_status esdm_write_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, const void* buffer);
esdm_status esdm_read_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, void* buffer);

esdm_status esdm_dataset_grids(esdm_dataset_t* dataset, int64_t* out_count, esdm_grid_t** out_grids) {
  eassert(dataset);
  eassert(out_count);
  eassert(out_grids);

  *out_count = dataset->gridCount;
  *out_grids = ea_memdup(dataset->grids, dataset->gridCount*sizeof*dataset->grids);

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

void esdmI_dataset_registerGrid(esdm_dataset_t* dataset, esdm_grid_t* grid) {
  if(dataset->gridCount + dataset->incompleteGridCount == dataset->gridSlotCount) {
    dataset->grids = ea_checked_realloc(dataset->grids, (dataset->gridSlotCount *= 2)*sizeof*dataset->grids);
  }
  eassert(dataset->gridCount + dataset->incompleteGridCount < dataset->gridSlotCount);

  dataset->grids[dataset->gridCount + dataset->incompleteGridCount++] = grid;
}

void esdmI_dataset_registerGridCompletion(esdm_dataset_t* dataset, esdm_grid_t* grid) {
  int64_t index = dataset->gridCount;
  while(index < dataset->gridCount + dataset->incompleteGridCount) {
    if(dataset->grids[index++] == grid) break;
  }
  eassert(index < dataset->gridCount + dataset->incompleteGridCount && "registerGridCompletion() must be called with an existing incomplete grid");

  esdm_grid_t* temp = dataset->grids[index];
  dataset->grids[index] = dataset->grids[dataset->gridCount];
  dataset->grids[dataset->gridCount] = temp;
  dataset->gridCount++, dataset->incompleteGridCount--;
}

int64_t esdmI_grid_coverRegionSize(const esdm_grid_t* grid, const esdmI_hypercube_t* region);	//returns the total size of the grid cells that intersect the region
void esdmI_grid_subgridCompleted(esdm_grid_t* grid);	//called by subgrids when their last cell is filled

void esdmI_grid_serialize(smd_string_stream_t* stream, const esdm_grid_t* grid);
esdm_status esdmI_grid_createFromString(const char* serializedGrid, esdm_grid_t** out_grid);

void esdmI_grid_destroy(esdm_grid_t* grid) {
  if(grid->grid) {
    for(int64_t i = esdmI_grid_cellCount(grid); i--; ) {
      if(grid->grid[i].subgrid) esdmI_grid_destroy(grid->grid[i].subgrid);
    }
    free(grid->grid);
  }
  for(int64_t dim = grid->dimCount; dim--; ) {
    if(grid->axes[dim].intervals != 1) free(grid->axes[dim].allBounds);
  }
  free(grid);
}
