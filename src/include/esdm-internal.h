/**
 * @file
 * @brief Internal ESDM functionality, not to be used by backends and plugins.
 *
 */

#ifndef ESDM_INTERNAL_H
#define ESDM_INTERNAL_H

#include <jansson.h>
#include <glib.h>
#include <inttypes.h>

#include <esdm-datatypes-internal.h>
#include <esdm-stream.h>
#include <esdm-debug.h>
#include <esdm.h>

// ESDM Core //////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Configuration //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the site configuration module.
 *
 * @param	[in] esdm   Pointer to esdm instance.
 * @return	Pointer to newly created configuration instance.
 */

esdm_config_t *esdm_config_init();

esdm_config_t *esdm_config_init_from_str(const char *str);

esdm_config_t* esdmI_getConfig();

esdm_status esdm_config_finalize(esdm_instance_t *esdm);

/**
 * Gathers ESDM configuration settings from multiple locations to build one configuration string.
 *
 */
char *esdm_config_gather();

/**
 *	Fetches backends
 *
 *
 */
esdm_config_backends_t *esdm_config_get_backends(esdm_instance_t *esdm);

esdm_config_backend_t *esdm_config_get_metadata_coordinator(esdm_instance_t *esdm);

esdm_instance_t* esdmI_esdm();

///////////////////////////////////////////////////////////////////////////////
// Modules /////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdm_modules_t* esdm_get_modules();

esdm_modules_t *esdm_modules_init(esdm_instance_t *esdm);

/**
 * Get a pointer to the backend with the highest estimated throughput.
 */
esdm_backend_t* esdm_modules_fastestBackend(esdm_modules_t* modules);

esdm_status esdm_modules_finalize();

/**
 * Make a recommendation on which data backend to use to store data for the given dataspace.
 *
 * @param [in] modules usually esdm->modules
 * @param [in] space the dataspace for which the recommendation is to be made
 * @param [out] out_moduleCount returns the number of recommended backends
 * @param [out] out_maxFragmentSize a max fragment size that is suitable for use with all the recommended backends (optional, may be NULL)
 * @return a freshly allocated array of *out_moduleCount backend pointers, must be free'd by the caller
 */
esdm_backend_t** esdm_modules_makeBackendRecommendation(esdm_modules_t* modules, esdm_dataspace_t* space, int64_t* out_moduleCount, int64_t* out_maxFragmentSize);

esdm_status esdm_modules_register();

esdm_status esdm_modules_get_by_type(esdm_module_type_t type, esdm_module_type_array_t **array);


///////////////////////////////////////////////////////////////////////////////
// Scheduler //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdm_scheduler_t *esdm_scheduler_init(esdm_instance_t *esdm);

esdm_status esdm_scheduler_finalize(esdm_instance_t *esdm);

esdm_status esdm_scheduler_status_init(io_request_status_t *status);

esdm_status esdm_scheduler_status_finalize(io_request_status_t *status);

/**
 * Calls to reads have to be completed before they can return to the application and are therefor blocking.
 *
 * Note: write is also blocking right now.
 *
 * @param[out] out_fillRegion Returns a pointer to a hypercube set that covers the region for which no data was found.
 *                            It's the callers' responsibility to either pass NULL or to destroy the hypercube set themselves.
 */

esdm_status esdm_scheduler_read_blocking(esdm_instance_t *esdm, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *memspace, esdmI_hypercubeSet_t** out_fillRegion, bool allowWriteback, bool requestIsInternal);

esdm_status esdm_scheduler_write_blocking(esdm_instance_t *esdm, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *memspace, bool requestIsInternal);

esdm_status esdmI_scheduler_writeFragmentBlocking(esdm_instance_t* esdm, esdm_fragment_t* fragment, bool requestIsInternal);
void esdmI_scheduler_writeFragmentNonblocking(esdm_instance_t* esdm, esdm_fragment_t* fragment, bool requestIsInternal, io_request_status_t* status);

esdm_status esdmI_scheduler_readSingleFragmentBlocking(esdm_instance_t* esdm, esdm_dataset_t* dataset, void* buffer, esdm_dataspace_t* memspace, esdm_fragment_t* fragment);

esdm_status esdm_scheduler_enqueue(esdm_instance_t *esdm, io_request_status_t *status, io_operation_t type, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *memspace);

esdm_status esdm_scheduler_wait(io_request_status_t *status);

/**
 * As `esdm_read()`, but also return the region that was filled with the fill value as a hypercube set.
 * If no fill value is set and any region without data is detected, this call will still return an error.
 *
 * @param [in] dataset TODO, currently a stub, we assume it has been identified/created before.... , json description?
 * @param [out] buf a contiguous memory region that shall be filled with the data from permanent storage
 * @param [in] memspace a dataspace that describes the location, size, and memory layout of the part of the data that is to be read
 * @param [out] out_fillRegion returns a new `esdmI_hypercubeSet_t*` that covers the region for which no data was found.
 *
 * @return status
 */
esdm_status esdmI_readWithFillRegion(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *memspace, esdmI_hypercubeSet_t** out_fillRegion);

///////////////////////////////////////////////////////////////////////////////
// Performance ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Queries backend for performance estimate for the given fragment.
 */
void fetch_performance_from_backend(gpointer key, gpointer value, gpointer user_data);

esdm_performance_t *esdm_performance_init(esdm_instance_t *esdm);

/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 */
esdm_status esdm_performance_recommendation(esdm_instance_t *esdm, esdm_fragment_t *in, esdm_fragment_t *out);

esdm_status esdm_performance_finalize();

esdm_readTimes_t esdmI_performance_read();
esdm_readTimes_t esdmI_performance_read_add(const esdm_readTimes_t* a, const esdm_readTimes_t* b);
esdm_readTimes_t esdmI_performance_read_sub(const esdm_readTimes_t* minuend, const esdm_readTimes_t* subtrahend);
void esdmI_performance_read_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_readTimes_t* start, const esdm_readTimes_t* end);  //start may be NULL

esdm_writeTimes_t esdmI_performance_write();
esdm_writeTimes_t esdmI_performance_write_add(const esdm_writeTimes_t* a, const esdm_writeTimes_t* b);
esdm_writeTimes_t esdmI_performance_write_sub(const esdm_writeTimes_t* minuend, const esdm_writeTimes_t* subtrahend);
void esdmI_performance_write_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_writeTimes_t* start, const esdm_writeTimes_t* end);  //start may be NULL

esdm_copyTimes_t esdmI_performance_copy();
esdm_copyTimes_t esdmI_performance_copy_add(const esdm_copyTimes_t* a, const esdm_copyTimes_t* b);
esdm_copyTimes_t esdmI_performance_copy_sub(const esdm_copyTimes_t* minuend, const esdm_copyTimes_t* subtrahend);
void esdmI_performance_copy_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_copyTimes_t* start, const esdm_copyTimes_t* end);  //start may be NULL

esdm_backendTimes_t esdmI_performance_backend();
esdm_backendTimes_t esdmI_performance_backend_add(const esdm_backendTimes_t* a, const esdm_backendTimes_t* b);
esdm_backendTimes_t esdmI_performance_backend_sub(const esdm_backendTimes_t* minuend, const esdm_backendTimes_t* subtrahend);
void esdmI_performance_backend_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_backendTimes_t* start, const esdm_backendTimes_t* end);  //start may be NULL

esdm_fragmentsTimes_t esdmI_performance_fragments();
esdm_fragmentsTimes_t esdmI_performance_fragments_add(const esdm_fragmentsTimes_t* a, const esdm_fragmentsTimes_t* b);
esdm_fragmentsTimes_t esdmI_performance_fragments_sub(const esdm_fragmentsTimes_t* minuend, const esdm_fragmentsTimes_t* subtrahend);
void esdmI_performance_fragments_print(FILE* stream, const char* linePrefix, const char* indentation, const esdm_fragmentsTimes_t* start, const esdm_fragmentsTimes_t* end);  //start may be NULL

//wrappers for the backend API functions that perform the time measurement to be retrieved via esdmI_performance_backend()
int esdmI_backend_finalize(esdm_backend_t * b);
int esdmI_backend_performance_estimate(esdm_backend_t * b, esdm_fragment_t *fragment, float *out_time);
float esdmI_backend_estimate_throughput (esdm_backend_t * b);
int esdmI_backend_fragment_create (esdm_backend_t * b, esdm_fragment_t *fragment);
int esdmI_backend_fragment_retrieve(esdm_backend_t * b, esdm_fragment_t *fragment);
int esdmI_backend_fragment_update (esdm_backend_t * b, esdm_fragment_t *fragment);
int esdmI_backend_fragment_delete (esdm_backend_t * b, esdm_fragment_t *fragment);
int esdmI_backend_fragment_metadata_create(esdm_backend_t * b, esdm_fragment_t *fragment, smd_string_stream_t* stream);
void* esdmI_backend_fragment_metadata_load(esdm_backend_t * b, esdm_fragment_t *fragment, json_t *metadata);
int esdmI_backend_fragment_metadata_free (esdm_backend_t * b, void * options);
int esdmI_backend_mkfs(esdm_backend_t * b, int format_flags);
int esdmI_backend_fsck(esdm_backend_t * b);
int esdmI_backend_fragment_write_stream_blocksize(esdm_backend_t * b, estream_write_t * state, void * cur_buf, size_t cur_offset, uint32_t cur_size);

double esdmI_backendOutputTime();
double esdmI_backendInputTime();
void esdmI_resetBackendIoTimes();

///////////////////////////////////////////////////////////////////////////////
// Container //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void esdmI_container_init(char const * name, esdm_container_t **out_container);

esdm_status esdm_container_open_md_load(esdm_container_t *c, char ** out_md, int * out_size);
esdm_status esdm_container_open_md_parse(esdm_container_t *c, char * md, int size);

void esdmI_container_register_dataset(esdm_container_t * c, esdm_dataset_t *dset);

/**
 * Destruct and free a container object.
 *
 * @param [in] container an existing container object that is no longer needed
 *
 * "_destroy" sounds too destructive, this will be renamed to esdm_container_close().
 */
esdm_status esdmI_container_destroy(esdm_container_t *container);


///////////////////////////////////////////////////////////////////////////////
// Dataset ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void esdm_dataset_init(esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset);

esdm_status esdm_dataset_open_md_load(esdm_dataset_t *dset, char ** out_md, int * out_size);
esdm_status esdm_dataset_open_md_parse(esdm_dataset_t *d, char * md, int size);

esdm_status esdmI_dataset_fragmentsCoveringRegion(esdm_dataset_t* dataset, esdmI_hypercube_t* region, int64_t* out_count, esdm_fragment_t*** out_fragments, esdmI_hypercubeSet_t** out_uncovered, bool* out_fullyCovered);

// Creates a fragment or returns an existing one.
// The dataset will own the fragment.
// In the case that an existing fragment is returned, the `buf` parameter is ignored.
// FIXME: check ownership of the memspace that is handed into this function, and define whether it makes a copy for the fragment or not
esdm_fragment_t* esdmI_dataset_createFragment(esdm_dataset_t* dataset, esdm_dataspace_t* memspace, void *buf, bool* out_newFragment);

esdm_fragment_t* esdmI_dataset_lookupFragmentForShape(esdm_dataset_t* dataset, esdm_dataspace_t* shape);

/**
 * Destruct and free a dataset object.
 *
 * @param [in] dataset an existing dataset object that is no longer needed
 *
 * @return status
 *
 * "_destroy" sounds too destructive, this will be renamed to esdm_dataset_close().
 */
esdm_status esdmI_dataset_destroy(esdm_dataset_t *dataset);


///////////////////////////////////////////////////////////////////////////////
// Dataspace //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Create a dataspace. Takes the shape in the form of an `esdmI_hypercube_t`.
 *
 * Similar to `esdm_dataspace_create()`, but takes an `esdmI_hypercube_t*` instead of a pair of `offset` and `size` arrays.
 *
 * @param [in] extends the logical shape of the dataspace that is to be created
 * @param [out] out_space returns a new dataspace object that needs to be destructed by the caller
 */
esdm_status esdmI_dataspace_createFromHypercube(esdmI_hypercube_t* extends, esdm_type_t type, esdm_dataspace_t** out_space);

/**
 * Get the logical extends covered by a dataspace in the form of an `esdmI_hypercube_t`.
 *
 * @param [in] space the dataspace to query
 * @param [out] out_extends returns a pointer to a hypercube with the extends of the dataspace, the caller is responsible to destroy the returned pointer
 *
 * @return ESDM_SUCCESS
 */
esdm_status esdmI_dataspace_getExtends(esdm_dataspace_t* space, esdmI_hypercube_t** out_extends);

/**
 * Set the logical extends covered by a dataspace in the form of an `esdmI_hypercube_t`.
 *
 * @param [in] space the dataspace to query
 * @param [in] extends a hypercube with the extends of the dataspace
 *
 * @return ESDM_SUCCESS
 */
esdm_status esdmI_dataspace_setExtends(esdm_dataspace_t* space, esdmI_hypercube_t* extends);


///////////////////////////////////////////////////////////////////////////////
// Fragment ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void esdmI_fragments_construct(esdm_fragments_t* me);
esdm_status esdmI_fragments_add(esdm_fragments_t* me, esdm_fragment_t* fragment) __attribute__((warn_unused_result));  //takes possession of the fragment, eventually calling `esdm_fragment_destroy()` on it when the `esdm_fragments_t` object is destructed
esdm_fragment_t* esdmI_fragments_lookupForShape(esdm_fragments_t* me, esdmI_hypercube_t* shape);
esdm_status esdmI_fragments_deleteAll(esdm_fragments_t* me);  //calls `esdmI_backend_fragment_delete()` and `esdmI_fragment_destroy()` on all fragments, leaving the fragment list empty on success
esdm_fragment_t** esdmI_fragments_makeSetCoveringRegion(esdm_fragments_t* me, esdmI_hypercube_t* region, int64_t* out_fragmentCount);  //caller is responsible to free the returned array
void esdmI_fragments_metadata_create(esdm_fragments_t* me, smd_string_stream_t* s);
void esdmI_fragments_purge(esdm_fragments_t* me); //this will `esdm_fragment_destroy()` all currently stored fragments
esdm_status esdmI_fragments_destruct(esdm_fragments_t* me);  //calls `esdm_fragment_destroy()` on its members, but does not invoke the `fragment_delete()` callback of the backend
void esdmI_fragments_getStats(int64_t* out_addedFragments, double* out_fragmentAddTime, int64_t* out_createdSets, double* out_setCreationTime);

void esdm_fragment_metadata_create(esdm_fragment_t *f, smd_string_stream_t * stream);
esdm_status esdmI_create_fragment_from_metadata(esdm_dataset_t *dset, json_t * json, esdm_fragment_t ** out);

/**
 * Create a new fragment.
 * If a non-NULL buf argument is supplied, the fragment only references the data, it does not take possession of the pointer.
 * It is the user's responsibility to ensure that the data remains alive as long as the fragment is loaded.
 *
 *  - Allocate process local memory structures.
 *
 *  - Takes possession of the dataspace. Do not modify or destroy it after calling this function!
 *
 *  - Does **not** register the fragment with the dataset. How the fragment will be owned is left to the discretion of the caller.
 *
 *
 *	A fragment is part of a dataset.
 *
 *	@return Pointer to new fragment.
 *
 */
esdm_status esdmI_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *memspace, void *buf, esdm_fragment_t **out_fragment);

///////////////////////////////////////////////////////////////////////////////
// Dysfunctional stuff ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Layout

/**
 * Initializes the init component by allocating and populating the esdm_layout
 * structure.
 *
 * @return Status
 */

esdm_layout_t *esdm_layout_init(esdm_instance_t *esdm);

/**
* Shutdown ESDM:
*  - finalize submodules
*  - free data structures
*
* @return Status
*/

esdm_status esdm_layout_finalize(esdm_instance_t *esdm);

/**
 * The layout reconstructor finds a reconstruction for subspace of a dataset.
 *
 * The reconstruction should take performance considerations into account.
 *
 * @return Status
 */

esdm_fragment_t *esdm_layout_reconstruction(esdm_dataset_t *dataset, esdm_dataspace_t *subspace);

/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 * @return Status
 */

esdm_status esdm_layout_recommendation(esdm_instance_t *esdm, esdm_fragment_t *in, esdm_fragment_t *out);


///////////////////////////////////////////////////////////////////////////////
// Backend (generic) //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdm_backend_t * esdmI_init_backend(char const * name, esdm_config_backend_t * config);

esdm_status esdm_backend_t_estimate_performance(esdm_backend_t *backend, int fragment);


///////////////////////////////////////////////////////////////////////////////
// auxiliary.c ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Print a detailed summary for the stat system call.
 */
void print_stat(struct stat sb);

int mkdir_recursive(const char *path);
int posix_recursive_remove(const char *path); //returns an error code

int ea_read_file(char *filepath, char **buf);

/**
 * Read while ensuring and retrying until len is read or error occured.
 */
int ea_read_check(int fd, char *buf, size_t len);

/**
 * Write while ensuring and retrying until len is written or error occured.
 */
int ea_write_check(int fd, char *buf, size_t len);


json_t *load_json(const char *str);

esdm_backend_t * esdmI_get_backend(char const * plugin_id);

void ea_generate_id(char *str, size_t length);  //str is a preexisting buffer with `length + 1` bytes
char* ea_make_id(size_t length);  //`malloc()`s a buffer for the result and calls through to `ea_generate_id()`

/**
 * Wrapper for malloc() that checks the result for a null-pointer.
 */
void* ea_checked_malloc(size_t size) __attribute__((alloc_size(1), malloc));

/**
 * Wrapper for calloc() that checks the result for a null-pointer.
 */
void* ea_checked_calloc(size_t nmemb, size_t size) __attribute__((alloc_size(1, 2), malloc));

/**
 * Wrapper for realloc() that checks the result for a null-pointer.
 */
void* ea_checked_realloc(void* ptr, size_t size) __attribute__((alloc_size(2), malloc));

/**
 * Wrapper for strdup() that checks the result for a null-pointer.
 */
char* ea_checked_strdup(const char* string);

/**
 * Wrapper for strndup() that checks the result for a null-pointer.
 */
char* ea_checked_strndup(const char* string, size_t n);

/**
 * Create a copy of an arbitrary memory buffer.
 * This is essentially a strdup() for non-string data.
 *
 * @param [in] data pointer to the bytes to copy
 * @param [in] size count of bytes to copy
 * @return freshly malloc'ed buffer containing a copy of the bytes, must be free'd by the caller
 */
void* ea_memdup(void* data, size_t size);

int ea_compute_hash_str(const char * str);
bool ea_is_valid_dataset_name(const char *str);

// timer functions
#ifdef ESM
typedef clock64_t timer;
#else
typedef struct timespec timer;
#endif

void ea_start_timer(timer *t1);
double ea_stop_timer(timer t1);
double ea_timer_subtract(timer number, timer subtract);

//data conversion
typedef void* (*ea_datatype_converter)(void* dest, const void* source, size_t sourceBytes);

/**
 * Return a memcpy()-like function that converts an array of sourceType into an array of destType.
 * The individual values are converted as with the respective C cast, preserving values where possible,
 * but possibly also wrapping around or losing precision if the original values cannot be represented exactly.
 *
 * Works only for some selected datatypes (noop conversions (`memcpy()`), `int8_t` through `uint64_t`, as well as `float` and `double`),
 * so be sure to check the return value against NULL and handle the error condition appropriately.
 *
 * Be aware that some conversions involve UB.
 * Especially out-of-range float/double conversion to integers can yield surprising results.
 */
ea_datatype_converter ea_converter_for_types(esdm_type_t destType, esdm_type_t sourceType);

///////////////////////////////////////////////////////////////////////////////
// esdmI_range_t //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//the resulting range may be empty
inline esdmI_range_t esdmI_range_intersection(esdmI_range_t a, esdmI_range_t b) {
  return (esdmI_range_t){
    .start = a.start > b.start ? a.start : b.start,
    .end = a.end < b.end ? a.end : b.end
  };
}

static inline bool esdmI_range_isEmpty(esdmI_range_t range) { return range.start >= range.end; }

static inline bool esdmI_range_equal(esdmI_range_t a, esdmI_range_t b) { return a.start == b.start && a.end == b.end; }

static inline int64_t esdmI_range_size(esdmI_range_t range) { return esdmI_range_isEmpty(range) ? 0 : range.end - range.start; }

void esdmI_range_print(esdmI_range_t range, FILE* stream);


///////////////////////////////////////////////////////////////////////////////
// esdmI_hypercube_t //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdmI_hypercube_t* esdmI_hypercube_makeDefault(int64_t dimensions); //initializes an empty hypercube with the given dimension count at (0, 0, ...)

esdmI_hypercube_t* esdmI_hypercube_make(int64_t dimensions, int64_t* offset, int64_t* size);

esdmI_hypercube_t* esdmI_hypercube_makeCopy(esdmI_hypercube_t* original);

//returns NULL if the intersection is empty
esdmI_hypercube_t* esdmI_hypercube_makeIntersection(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

bool esdmI_hypercube_isEmpty(esdmI_hypercube_t* cube);

//These two functions implement exactly the same algorithm,
//i.e. it is guaranteed that both return the same value when given the same shape.
//
//This is *not* a cryptographic hash.
uint32_t esdmI_hypercube_hash(const esdmI_hypercube_t* me);
uint32_t esdmI_hypercube_hashOffsetSize(int64_t dimCount, const int64_t* offset, const int64_t* size);

bool esdmI_hypercube_doesIntersect(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

//Touch means no overlap is allowed.
//The two hypercubes must share a single border such that there is at least one pair of voxels next to each other
//where one voxel belongs to hypercube `a` while the other belongs to hypercube `b`.
bool esdmI_hypercube_touches(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

//Returns the count of grid points that are shared between the two hypercubes.
//This method is cheaper to call than the equivalent *_makeIntersection(), *_size(), *_destroy() combo;
//it avoids the overhead of actually creating the intersection hypercube.
int64_t esdmI_hypercube_overlap(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

bool esdmI_hypercube_equal(const esdmI_hypercube_t* a, const esdmI_hypercube_t* b);

//Returns a value between 0.0 and 1.0 that is a measure of how similar the shapes of the hypercubes are.
//The positions of the two cubes are irrelevant, only size and shape matter.
//The algorithm basically translates both hypercubes to start at (0, ..., 0) and then checks the volume of the intersection against the volume of the two hypercubes.
//A return value of 1.0 means that the two hypercubes are identical modulo translation, a value of 0.0 is only achieved if one of the hypercubes is empty.
double esdmI_hypercube_shapeSimilarity(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

static inline int64_t esdmI_hypercube_dimensions(const esdmI_hypercube_t* cube) { return cube->dims; }

/**
 * Return the shape of the hypercube as an offset and a size vector.
 *
 * @param [in] cube the hypercube to get the shape of
 * @param [out] out_offset array of size cube->dims that will be filled with the offset vector components
 * @param [out] out_size array of size cube->dims that will be filled with the size vector components
 */
void esdmI_hypercube_getOffsetAndSize(const esdmI_hypercube_t* cube, int64_t* out_offset, int64_t* out_size);

int64_t esdmI_hypercube_size(esdmI_hypercube_t* cube);

void esdmI_hypercube_print(esdmI_hypercube_t* cube, FILE* stream);

void esdmI_hypercube_destroy(esdmI_hypercube_t* cube);


///////////////////////////////////////////////////////////////////////////////
// esdmI_hypercubeList_t //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool esdmI_hypercubeList_doesIntersect(esdmI_hypercubeList_t* list, esdmI_hypercube_t* cube);

bool esdmI_hypercubeList_doesCoverFully(esdmI_hypercubeList_t* list, esdmI_hypercube_t* cube);

/**
 * This function returns a number of minimal subsets of the cubes contained within the hypercubeList.
 * The selection of the subsets is probabilistic as any complete algorithm I could think of would have had exponential complexity.
 * The probabilistic solution allows the caller to specify how much effort should be spent in finding minimal subsets.
 * With the probabilistic solution, the function creates as many subsets as indicated by `*inout_setCount`,
 * and returns the number of actually found subsets in the same memory location.
 * The returned value may be lower than the given value because a subset may be found several times.
 *
 * The probabilistic algorithm is written in such a way that it will find small minimal subsets more easily than subsets that contain more cubes.
 *
 * The complexity of the algorithm is `O(*inout_setCount * list->count^2)`.
 *
 * @param [in] list the hypercube list from which to select subsets
 * @param [inout] inout_setCount on input the requested amount of subsets that are to be generated, on output the actual number of distinct subsets retured in `out_subsets`
 * @param [out] out_subsets a 2D array of dimensions `out_subsets[*inout_setCount][list->count]`
 *              that will be used to store the flags which cube is selected for which subset;
 *              if `out_subsets[i][j]` is true then the cube with index `j` is selected for the subset `i`
 */
void esdmI_hypercubeList_nonredundantSubsets_internal(esdmI_hypercubeList_t* list, int64_t count, int64_t* inout_setCount, uint8_t (*out_subsets)[count]);
//wrapper to encapsulate redundant passing of `list->count`
#define esdmI_hypercubeList_nonredundantSubsets(list, inout_setCount, out_subsets) do {\
  esdmI_hypercubeList_t* l = list;\
  esdmI_hypercubeList_nonredundantSubsets_internal(l, l->count, inout_setCount, out_subsets);\
} while(false)

void esdmI_hypercubeList_print(esdmI_hypercubeList_t* list, FILE* stream);  //for debugging purposes

///////////////////////////////////////////////////////////////////////////////
// esdmI_hypercubeSet_t ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdmI_hypercubeSet_t* esdmI_hypercubeSet_make();  //convenience function to construct a heap allocated object
void esdmI_hypercubeSet_construct(esdmI_hypercubeSet_t* me);  //no allocation, initialization only

//the returned list and the pointers it contains only remain valid as long the hypercubeSet is not changed in any way
esdmI_hypercubeList_t* esdmI_hypercubeSet_list(esdmI_hypercubeSet_t* me);

bool esdmI_hypercubeSet_isEmpty(esdmI_hypercubeSet_t* me);  //checks for logical emptiness, may remove empty hypercubes from the set

int64_t esdmI_hypercubeSet_count(esdmI_hypercubeSet_t* me);

void esdmI_hypercubeSet_add(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube);

void esdmI_hypercubeSet_subtract(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube);

void esdmI_hypercubeSet_destruct(esdmI_hypercubeSet_t* me); //counterpart to esdmI_hypercubeSet_construct()
void esdmI_hypercubeSet_destroy(esdmI_hypercubeSet_t* me);  //counterpart to esdmI_hypercubeSet_make()

///////////////////////////////////////////////////////////////////////////////
// esdmI_hypercubeNeighbourManager_t //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdmI_hypercubeNeighbourManager_t* esdmI_hypercubeNeighbourManager_make(int64_t dimensions);  //all hypercubes added to this manager need to have the same rank

//the returned list and the pointers it contains only remain valid as long the hypercubeNeighbourManager is not changed in any way
esdmI_hypercubeList_t* esdmI_hypercubeNeighbourManager_list(esdmI_hypercubeNeighbourManager_t* me);

//takes possession of the cube, the caller needs to make a copy if necessary
void esdmI_hypercubeNeighbourManager_pushBack(esdmI_hypercubeNeighbourManager_t* me, esdmI_hypercube_t* cube);

//Returns a pointer to an array that lists the neighbour indices of the given cube, this array has `*out_neighbourCount` entries.
//Callers must provide a non-NULL value for `out_neighbourCount`.
int64_t* esdmI_hypercubeNeighbourManager_getNeighbours(esdmI_hypercubeNeighbourManager_t* me, int64_t index, int64_t* out_neighbourCount);

void esdmI_hypercubeNeighbourManager_destroy(esdmI_hypercubeNeighbourManager_t* me);

#ifdef HAVE_SCIL
SCIL_Datatype_t ea_esdm_datatype_to_scil(smd_basic_type_t type);
#endif

#endif
