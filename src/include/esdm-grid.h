#ifndef ESDM_GRID_H
#define ESDM_GRID_H

#include <esdm-datatypes-internal.h>

#include <stdbool.h>

/**
 * esdm_grid_create(), esdm_grid_createSimple()
 *
 * Create a grid object.
 *
 * The grid is constructed to contain only a single cell,
 * which will cover the entire domain of the grid (the hyperbox defined by the offset and size arrays).
 * The normal use case is to subsequently subdivide one or more axes using either `esdm_grid_subdivideFixed()` or `esdm_grid_subdivideFlexible()` before using the grid in an I/O operation.
 *
 * @param[in] dataset the dataset to which the grid should belong
 * @param[in] dimCount the dimensionality of the grid's domain, must match the dimension count of the dataset
 * @param[in] offset an array of `dimCount` elements, contains the starting coordinate of the grid's domain, in the case of `esdm_grid_createSimple()` the origin is assumed
 * @param[in] size an array of `dimCount` elements, contains the count of data points within the domain in each dimension
 * @param[out] out_grid a pointer to the newly constructed grid, or NULL in case of an error
 *
 * The grid will be owned by the dataset, it will remain valid until the dataset is closed.
 *
 * Grids are always in one of three possible states:
 *
 *  1. Axis definition state.
 *     This is the initial state, it allows all actions to be performed on the grid, but some actions put the grid in one of the other states.
 *
 *  2. Fixed axes state.
 *     The Axes of the grid are fixed, and all calls to `esdm_grid_subdivide*()` will result in an error.
 *     However, subgrids may still be added to the grid.
 *
 *     This state is entered when a subgrid is created via `esdm_grid_createSubgrid()`.
 *
 *  3. Fixed structure state.
 *     In addition to the axes, the subgrid structure of the grid is fixed.
 *     All calls to `esdm_grid_createSubgrid()` will result in an error.
 *
 *     This state is entered by any call that persists or communicates a grid.
 *     Committing a dataset will put all grids of the dataset into this state, as will do any call to `esdm_mpi_grid_bcast()`.
 *
 * Once a grid has entered a higher state, it is impossible to enter any lower states.
 *
 * @return a status code that identifies the cause of an error
 */
esdm_status esdm_grid_create(esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size, esdm_grid_t** out_grid);
esdm_status esdm_grid_createSimple(esdm_dataset_t* dataset, int64_t dimCount, int64_t* size, esdm_grid_t** out_grid);

/**
 * esdm_grid_createSubgrid()
 *
 * Subdivide a single cell of the parent grid, building a hierarchy of grids.
 *
 * @param[in] parent the grid that will contain the child grid as one of its cells
 * @param[in] index an array of `dimCount` elements, contains the cell coordinates of the cell that is to be subdivided
 * @param[out] out_child on success, this returns a pointer to the new grid
 *
 * The child grid will have a domain that matches with the extends of the parent grid cell.
 * As with `esdm_grid_create()`, the child grid will initially contain a single cell, only.
 * Consequently, a call to `esdm_grid_createSubgrid()` should always be followed by a call to `esdm_grid_subdivide*()` for the child grid.
 *
 * This puts the parent grid into fixed axis state, further calls to `esdm_grid_subdivide*()` will result in an error.
 * It is not possible to call this method in fixed structure state.
 *
 * The grid will be owned by the dataset, it will remain valid until the dataset is closed.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the given index does not exist, `ESDM_INVALID_STATE_ERROR` if there is already data associated with the grid cell at `index`.
 */
esdm_status esdm_grid_createSubgrid(esdm_grid_t* parent, int64_t* index, esdm_grid_t** out_child);

/**
 * esdm_grid_subdivideFixed()
 *
 * Subdivide a dimension into slices of a fixed size.
 *
 * @param[inout] grid the grid object to modify
 * @param[in] dim index of the dimension that is to be subdivided
 * @param[in] size thickness of each slice in the given direction
 * @param[in] allowIncomplete boolean that signals that the last slice is allowed to be smaller than the given size
 *
 * If `!allowIncomplete`, it is an error to specify a `size` that does not divide the size of the grid's domain in the given dimension.
 * If `allocIncomplete`, the last slice will be defined smaller than `size` if the given size does not divide the grid's domain in the given dimension.
 *
 * This method can only be called in axis definition state.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the arguments are inconsistent with the given grid, `ESDM_INVALID_STATE_ERROR` if the grid has already been used in an I/O operation
 */
esdm_status esdm_grid_subdivideFixed(esdm_grid_t* grid, int64_t dim, int64_t size, bool allowIncomplete);

/**
 * esdm_grid_subdivideFlexible()
 *
 * Subdivide a dimension into a given number of slices.
 *
 * @param[inout] grid the grid object to modify
 * @param[in] dim index of the dimension that is to be subdivided
 * @param[in] count number of slices to create
 *
 * The bounds of the slices will be evenly distributed across the dimension,
 * in the case that the `count` does not divide the domain's size in the given dimension, some slice sizes will be rounded down while others will be rounded up.
 * Use `esdm_grid_cell_extends()` to inquire the actual size of a grid cell.
 *
 * This method can only be called in axis definition state.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the arguments are inconsistent with the given grid, `ESDM_INVALID_STATE_ERROR` if the grid has already been used in an I/O operation
 */
esdm_status esdm_grid_subdivideFlexible(esdm_grid_t* grid, int64_t dim, int64_t count);

/**
 * esdm_grid_subdivide()
 *
 * Subdivide a dimension with a given list of bounds.
 *
 * @param[inout] grid the grid object to modify
 * @param[in] dim index of the dimension that is to be subdivided
 * @param[in] count number of slices to create
 * @param[in] bounds array of `count + 1` bounds, this includes the outer bounds of the grid
 *
 * This is the big gun among the `esdm_grid_subdivide*()` methods, allowing the caller to specify exactly which bounds are to be used.
 * Use this when the regular subdivision done by the other two methods does not cut it.
 *
 * This method can only be called in axis definition state.
 *
 * The contents of the `bounds` array must satisfy the following conditions:
 *   * `bounds[0]` must equal the starting bound of the grid in the given dimension
 *   * `bounds[count]` must equal the ending bound of the grid in the given dimension (this bound has exclusive semantics)
 *   * `bounds[i+1] > `bounds[i]`
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the bounds array does not satisfy one of the conditions above, or if the `dim` value is illegal, `ESDM_INVALID_STATE_ERROR` if the grid has already been used in an I/O operation
 */
esdm_status esdm_grid_subdivide(esdm_grid_t* grid, int64_t dim, int64_t count, int64_t* bounds);

/**
 * esdm_grid_cellSize()
 *
 * Inquire the bounding hyperbox of a single grid cell.
 *
 * @param[in] grid the grid to inquire
 * @param[in] cellIndex array of `dimCount` elements that contains the grid coordinates of the cell that is to be inquired
 * @param[out] out_offset array of `dimCount` elements that will be set to the starting coordinates of the cell
 * @param[out] out_size array of `dimCount` elements that will be set to the size of the cell
 *
 * A grid has two independent sets of coordinates:
 *   - the domain coordinates which are defined when the grid object is created
 *   - the grid coordinates which are defined by subsequent calls to `esdm_grid_subdivide_*()`
 * The later coordinates always put the first grid cell at the origin, and adding/subtracting `1` to/from any coordinate yields the grid coordinates of a neighbouring grid cell.
 * This functions basically translates the grid coordinates into the domain coordinates of the grid cell's bounding hyperbox.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the given grid cell does not exist
 */
esdm_status esdm_grid_cellSize(const esdm_grid_t* grid, int64_t* cellIndex, int64_t* out_offset, int64_t* out_size);

/**
 * esdm_grid_cellExtends()
 *
 * Inquire the bounding hyperbox of a single grid cell.
 *
 * @param[in] grid the grid to inquire
 * @param[in] cellIndex array of `dimCount` elements that contains the grid coordinates of the cell that is to be inquired
 * @param[out] out_extends pointer to hypercube with the domain coordinates of the grid cell, the caller is responsible to destroy this properly
 *
 * A grid has two independent sets of coordinates:
 *   - the domain coordinates which are defined when the grid object is created
 *   - the grid coordinates which are defined by subsequent calls to `esdm_grid_subdivide_*()`
 * The later coordinates always put the first grid cell at the origin, and adding/subtracting `1` to/from any coordinate yields the grid coordinates of a neighbouring grid cell.
 * This functions basically translates the grid coordinates into the domain coordinates of the grid cell's bounding hyperbox.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the given grid cell does not exist
 */
esdm_status esdm_grid_cellExtends(const esdm_grid_t* grid, int64_t* cellIndex, esdmI_hypercube_t** out_extends);

/**
 * esdm_grid_getBound()
 *
 * Inquire a single bound from the grid.
 *
 * @param[in] grid the grid to inquire
 * @param[in] dim the dimension to inquire
 * @param[in] index the index of the bound to inquire
 * @param[out] out_bound the value of the bound
 *
 * The index is an integer in the interval `[0, sliceCount]`, both inclusive, where `sliceCount` is the number of slices in the given dimension.
 * An index of `0` will return the offset that was given for the dimension in the `esdm_grid_create()` call,
 * an index of `sliceCount` will return the sum of offset and size.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the given bound does not exist
 */
esdm_status esdm_grid_getBound(const esdm_grid_t* grid, int64_t dim, int64_t index, int64_t* out_bound);

/**
 * esdm_write_grid()
 *
 * Provide the data for a grid cell.
 *
 * @param[in] grid defines the dataset as well as the legal shapes of cells that may be written to
 * @param[in] memspace defines which cell is to be written as well as the data layout within the buffer
 * @param[in] buffer pointer to the first data element that is to be written
 *
 * The extends of the memspace must match exactly a single (sub-)cell of the grid.
 * It does not matter whether a parent or child grid is used in this call,
 * `esdm_write_grid()` will determine both the global parent and the local grid cell containing child internally.
 *
 * This puts the grid into fixed axis state unless the grid is already in a higher state.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the `memspace` does not correspond to a single cell exactly, `ESDM_INVALID_STATE_ERROR` if the grid cell contains a subgrid
 */
esdm_status esdm_write_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, void* buffer);

/**
 * esdm_read_grid()
 *
 * Read data from a single grid cell into memory.
 *
 * @param[in] grid defines the dataset as well as the legal shapes of cells that may be read
 * @param[in] memspace defines which cell is to be read as well as the data layout within the buffer
 * @param[out] buffer pointer to the first data element that is to be read
 *
 * The extends of the memspace must match exactly a single (sub-)cell of the grid.
 * It does not matter whether a parent or child grid is used in this call,
 * `esdm_read_grid()` will determine both the global parent and the local grid cell containing child internally.
 *
 * It is not necessary for the grid cell to actually contain data itself,
 * ESDM will read data provided by another grid or gridless write in this case.
 * However, it may fill the grid cell with the resulting data.
 *
 * If the read operation succeeds, the grid will be at least in fixed axis state.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the `memspace` does not correspond to a single cell exactly, `ESDM_INVALID_STATE_ERROR` if the grid cell contains a subgrid
 */
//TODO: Specify under which conditions the data is written back.
esdm_status esdm_read_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, void* buffer);

/**
 * esdm_dataset_grids()
 *
 * Inquire the available grids from a dataset.
 *
 * This will only return completed grids, i.e. grids which contain data for their entire domain.
 *
 * @param[in] dataset the dataset to query
 * @param[out] out_count the number of available grids
 * @param[out, optional] out_grids returns an array of pointers to the available grids
 *
 * The array that is returned in `*out_grids` is malloc'ed by `esdm_dataset_grids()`, it is the callers responsibility to call `free()` on the array.
 * The grids themselves, however, still belong to the dataset and remain valid until the dataset is closed.
 *
 * All the returned grids will contain data for their entire domain,
 * and they will all be at least in fixed axis state.
 *
 * If `NULL` is passed as the `out_grids` argument, the function will still return the count of grids via the `out_count` parameter.
 * It is a contract violation to pass `NULL` as the `out_count` parameter.
 *
 * @return `ESDM_SUCCESS` on success
 */
esdm_status esdm_dataset_grids(esdm_dataset_t* dataset, int64_t* out_count, esdm_grid_t*** out_grids);

/**
 * esdm_gridIterator_create()
 *
 * Obtain an iterator that will recursively walk all cells in the grid.
 *
 * This function exists for the benefit of readers that do not need to read their input in a specific order,
 * and can gain significant performance benefits by using a domain decomposition that matches what is actually stored on disk.
 *
 * @param[in] grid the grid to walk
 * @param[out] out_iterator an iterator that can subsequently passed to `esdm_gridIterator_next()`
 *
 * It is an error to call this on a grid that does not have data for its entire domain
 * (this implies at least fixed axis state).
 * Any grid that has been returned by `esdm_dataset_grids()` will satisfy this condition.
 *
 * The returned object must be disposed off with a call to `esdm_gridIterator_destroy()`.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_STATE_ERROR` if the grid's data is not complete
 */
esdm_status esdm_gridIterator_create(esdm_grid_t* grid, esdm_gridIterator_t** out_iterator);

/**
 * esdm_gridIterator_next()
 *
 * Obtain a dataspace for a chunk of data.
 *
 * @param[inout] inout_iterator the iterator to advance
 * @param[in] increment the number of grid cells to advance, must not be negative
 * @param[out] out_dataspace a new dataspace that covers the selected grid cell, ready to be passed to `esdm_read_grid()`, NULL when the iterator has been advanced past the last grid cell
 *
 * The caller is responsible to `esdm_dataspace_destroy()` the resulting dataspace.
 *
 * The increment argument is useful for parallel applications, as it allows each process to iterate over its own subset of grid cells.
 *
 * `esdm_gridIterator_next()` may choose to replace the iterator pointer at `*inout_iterator` with a different one (this is intended to support subgrids),
 * but that is of no importance to the caller.
 * When such a replacement happens, the `esdm_gridIterator_new()` implementation takes care to do so fully transparently.
 *
 * After the last grid cell has been returned, `esdm_gridIterator_next()` will return `ESDM_SUCCESS` and set both `*out_dataspace` and `*inout_iterator` to NULL to signal the condition.
 * As such, it is not necessary to call `esdm_gridIterator_destroy()` if the iterator is advanced over the end of the grid,
 * `esdm_gridIterator_destroy()` exists to dispose off incompletely advanced iterators, only.
 *
 * @return `ESDM_SUCCESS` on success
 */
esdm_status esdm_gridIterator_next(esdm_gridIterator_t** inout_iterator, int64_t increment, esdm_dataspace_t** out_dataspace);

/**
 * esdm_gridIterator_destroy()
 *
 * @param[in] iterator the iterator to destroy, may be NULL
 *
 * Destroy and deallocate a grid iterator.
 * In the case of NULL argument, this is a noop.
 */
void esdm_gridIterator_destroy(esdm_gridIterator_t* iterator);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal API ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


void esdmI_dataset_registerGrid(esdm_dataset_t* dataset, esdm_grid_t* grid);
void esdmI_dataset_registerGridCompletion(esdm_dataset_t* dataset, esdm_grid_t* grid);

//Compute the total size of the grid cells that intersect the region.
//This assumes that the grid is complete, i.e. it does not waste time checking presence of fragments and subgrids.
int64_t esdmI_grid_coverRegionSize(const esdm_grid_t* grid, const esdmI_hypercube_t* region);

//Compute the total size of data that would need to be fetched to cover the region, minus the cover region size.
//TODO: Make this recurs into subgrids to get a precise result, currently this may overestimate the overhead.
int64_t esdmI_grid_coverRegionOverhead(esdm_grid_t* grid, esdmI_hypercube_t* region);

//Recursively list all fragments in the grid that intersect with the given region.
//There must be no empty cells within the provided region, and there must be an overlap with the region.
esdm_status esdmI_grid_fragmentsInRegion(esdm_grid_t* grid, esdmI_hypercube_t* region, int64_t* out_fragmentCount, esdm_fragment_t*** out_fragments);

//For use by esdm_mpi and storing as metadata.
//This puts the grid into fixed structure state.
void esdmI_grid_serialize(smd_string_stream_t* stream, esdm_grid_t* grid);  //the resulting stream is in JSON format
esdm_status esdmI_grid_createFromJson(json_t* json, esdm_dataset_t* dataset, esdm_grid_t* parent, esdm_grid_t** out_grid);
esdm_status esdmI_grid_createFromString(const char* serializedGrid, esdm_grid_t** out_grid);
//These two functions must be called with a string/json that was created from a copy of the grid, i.e. the grid IDs must match.
esdm_status esdmI_grid_mergeWithJson(esdm_grid_t* grid, json_t* json);
esdm_status esdmI_grid_mergeWithString(esdm_grid_t* grid, const char* serializedGrid);

//Access the ID of a grid.
//Returns NULL if the grid is not in fixed structure state yet (i.e. if it has not been (de-)serialized yet).
//The returned pointer is owned by the grid, do not modify or deallocate it.
const char* esdmI_grid_id(esdm_grid_t* grid);
bool esdmI_grid_matchesId(const esdm_grid_t* grid, const char* id);

void esdmI_grid_destroy(esdm_grid_t* grid);	//must only be called by the owning dataset

#endif
