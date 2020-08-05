#ifndef ESDM_GRID_H
#define ESDM_GRID_H

#include <esdm-datatypes-internal.h>

#include <stdbool.h>

typedef struct esdm_grid_t esdm_grid_t;
typedef struct esdm_gridIterator_t esdm_gridIterator_t;

/*
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
 * @return a status code that identifies the cause of an error
 */
esdm_status esdm_grid_create(esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size, esdm_grid_t** out_grid);
esdm_status esdm_grid_createSimple(esdm_dataset_t* dataset, int64_t dimCount, int64_t* size, esdm_grid_t** out_grid);

/*
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
 * This function is considered an I/O call on the parent grid, further `esdm_grid_subdivide*()` calls on the parent will result in an error.
 *
 * The grid will be owned by the dataset, it will remain valid until the dataset is closed.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the given index does not exist, `ESDM_INVALID_STATE_ERROR` if there is already data associated with the grid cell at `index`.
 */
esdm_status esdm_grid_createSubgrid(esdm_grid_t* parent, int64_t* index, esdm_grid_t** out_child);

/*
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
 * This function must not be called once the grid has been used in an I/O or MPI call.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the arguments are inconsistent with the given grid, `ESDM_INVALID_STATE_ERROR` if the grid has already been used in an I/O operation
 */
esdm_status esdm_grid_subdivideFixed(esdm_grid_t* grid, int64_t dim, int64_t size, bool allowIncomplete);

/*
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
 * This function must not be called once the grid has been used in an I/O or MPI call.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the arguments are inconsistent with the given grid, `ESDM_INVALID_STATE_ERROR` if the grid has already been used in an I/O operation
 */
esdm_status esdm_grid_subdivideFlexible(esdm_grid_t* grid, int64_t dim, int64_t count);

/*
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

/*
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

/*
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

/*
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
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the `memspace` does not correspond to a single cell exactly, `ESDM_INVALID_STATE_ERROR` if the grid cell contains a subgrid
 */
esdm_status esdm_write_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, const void* buffer);

/*
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
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_ARGUMENT_ERROR` if the `memspace` does not correspond to a single cell exactly, `ESDM_INVALID_STATE_ERROR` if the grid cell contains a subgrid
 */
//TODO: Specify under which conditions the data is written back.
esdm_status esdm_read_grid(esdm_grid_t* grid, esdm_dataspace_t* memspace, void* buffer);

/*
 * esdm_dataset_grids()
 *
 * Inquire the available grids from a dataset.
 *
 * This will only return completed grids, i.e. grids which contain data for their entire domain.
 *
 * @param[in] dataset the dataset to query
 * @param[out] out_count the number of available grids
 * @param[out, optional] out_grids return the pointer to the first element of an array containing the available grids
 *
 * The array that is returned in `*out_grids` is malloc'ed by `esdm_dataset_grids()`, it is the callers responsibility to call `free()` on the array.
 * The grids themselves, however, still belong to the dataset and remain valid until the dataset is closed.
 *
 * All the returned grids will contain data for their entire domain.
 *
 * If `NULL` is passed as the `out_grids` argument, the function will still return the count of grids via the `out_count` parameter.
 * It is a contract violation to pass `NULL` as the `out_count` parameter.
 *
 * @return `ESDM_SUCCESS` on success
 */
esdm_status esdm_dataset_grids(esdm_dataset_t* dataset, int64_t* out_count, esdm_grid_t** out_grids);

/*
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
 * It is an error to call this on a grid that does not have data for its entire domain.
 * Any grid that has been returned by `esdm_dataset_grids()` will satisfy this condition.
 *
 * The returned object must be disposed off with a call to `esdm_gridIterator_destroy()`.
 *
 * @return `ESDM_SUCCESS` on success, `ESDM_INVALID_STATE_ERROR` if the grid's data is not complete
 */
esdm_status esdm_gridIterator_create(esdm_grid_t* grid, esdm_gridIterator_t** out_iterator);

/*
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

/*
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

int64_t esdmI_grid_coverRegionSize(const esdm_grid_t* grid, const esdmI_hypercube_t* region);	//returns the total size of the grid cells that intersect the region

void esdmI_grid_subgridCompleted(esdm_grid_t* grid);	//called by subgrids when their last cell is filled

//For use by esdm_mpi and storing as metadata.
void esdmI_grid_serialize(smd_string_stream_t* stream, const esdm_grid_t* grid);
esdm_status esdmI_grid_createFromString(const char* serializedGrid, esdm_grid_t** out_grid);

void esdmI_grid_destroy(esdm_grid_t* grid);	//must only be called by the owning dataset

#endif
