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
#include <esdm-debug.h>
#include <esdm.h>

// ESDM Core //////////////////////////////////////////////////////////////////

// Configuration

/**
 * Initializes the site configuration module.
 *
 * @param	[in] esdm   Pointer to esdm instance.
 * @return	Pointer to newly created configuration instance.
 */

esdm_config_t *esdm_config_init();

esdm_config_t *esdm_config_init_from_str(const char *str);

esdm_status esdm_config_finalize(esdm_instance_t *esdm);

void esdm_dataset_init(esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset);


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

// Datatypes

// Modules
esdm_modules_t *esdm_modules_init(esdm_instance_t *esdm);

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

// I/O Scheduler

/**
 * Initialize scheduler component:
 *     * setup a thread pool
 *     * allow global and local limits
 *
 *     use globale limit only if ESDM_ACCESSIBILITY_GLOBAL is set    (data_accessibility_t enum)
 *
 *
 */

esdm_backend_t * esdmI_init_backend(char const * name, esdm_config_backend_t * config);

esdm_scheduler_t *esdm_scheduler_init(esdm_instance_t *esdm);

esdm_status esdm_scheduler_finalize(esdm_instance_t *esdm);

esdm_status esdm_scheduler_status_init(io_request_status_t *status);

esdm_status esdm_scheduler_status_finalize(io_request_status_t *status);

/**
 * Calls to reads have to be completed before they can return to the application and are therefor blocking.
 * Use esdm_scheduler_process_blocking from functions in the application facing API to process blocking scheduling.
 *
 * Note: write is also blocking right now.
 */

esdm_status esdm_scheduler_process_blocking(esdm_instance_t *esdm, io_operation_t type, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *subspace);

esdm_status esdm_scheduler_enqueue(esdm_instance_t *esdm, io_request_status_t *status, io_operation_t type, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *subspace);

esdm_status esdm_scheduler_wait(io_request_status_t *status);

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


/**
 * Queries backend for performance estimate for the given fragment.
 */

void fetch_performance_from_backend(gpointer key, gpointer value, gpointer user_data);

// Performance Model

esdm_performance_t *esdm_performance_init(esdm_instance_t *esdm);

/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 */

esdm_status esdm_performance_recommendation(esdm_instance_t *esdm, esdm_fragment_t *in, esdm_fragment_t *out);

esdm_status esdm_performance_finalize();

// Backend (generic)

esdm_status esdm_backend_t_estimate_performance(esdm_backend_t *backend, int fragment);

///////////////////////////////////////////////////////////////////////////////
// UTILS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// auxiliary.c ////////////////////////////////////////////////////////////////

/**
 * Print a detailed summary for the stat system call.
 */

void print_stat(struct stat sb);

int mkdir_recursive(const char *path);

void posix_recursive_remove(const char *path);

int read_file(char *filepath, char **buf);

/**
 * Read while ensuring and retrying until len is read or error occured.
 */

int read_check(int fd, char *buf, size_t len);

/**
 * Write while ensuring and retrying until len is written or error occured.
 */

int write_check(int fd, char *buf, size_t len);



json_t *load_json(const char *str);

void esdmI_container_init(char const * name, esdm_container_t **out_container);

esdm_status esdm_dataset_open_md_load(esdm_dataset_t *dset, char ** out_md, int * out_size);
esdm_status esdm_dataset_open_md_parse(esdm_dataset_t *d, char * md, int size);

esdm_status esdm_container_open_md_load(esdm_container_t *c, char ** out_md, int * out_size);
esdm_status esdm_container_open_md_parse(esdm_container_t *c, char * md, int size);

esdm_status esdmI_dataset_lookup_fragments(esdm_dataset_t *dataset, esdm_dataspace_t *space, int *out_frag_count, esdm_fragment_t ***out_fragments);

void esdmI_container_register_dataset(esdm_container_t * c, esdm_dataset_t *dset);
esdm_status esdmI_create_fragment_from_metadata(esdm_dataset_t *dset, json_t * json, esdm_fragment_t ** out);
void esdmI_fragments_metadata_create(esdm_dataset_t *d, smd_string_stream_t * s);

/**
 * Create a new fragment.
 *
 *  - Allocate process local memory structures.
 *
 *
 *	A fragment is part of a dataset.
 *
 *	@return Pointer to new fragment.
 *
 */

esdm_status esdmI_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, void *buf, esdm_fragment_t **out_fragment);

esdm_backend_t * esdmI_get_backend(char const * plugin_id);


void ea_generate_id(char *str, size_t length);

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


/**
 * Destruct and free a container object.
 *
 * @param [in] container an existing container object that is no longer needed
 *
 * "_destroy" sounds too destructive, this will be renamed to esdm_container_close().
 */

esdm_status esdmI_container_destroy(esdm_container_t *container);

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

//the resulting range may be empty
inline esdmI_range_t esdmI_range_intersection(esdmI_range_t a, esdmI_range_t b) {
  return (esdmI_range_t){
    .start = a.start > b.start ? a.start : b.start,
    .end = a.end < b.end ? a.end : b.end
  };
}

inline bool esdmI_range_isEmpty(esdmI_range_t range) { return range.start >= range.end; }

inline int64_t esdmI_range_size(esdmI_range_t range) { return esdmI_range_isEmpty(range) ? 0 : range.end - range.start; }

void esdmI_range_print(esdmI_range_t range, FILE* stream);

esdmI_hypercube_t* esdmI_hypercube_makeDefault(int64_t dimensions); //initializes an empty hypercube with the given dimension count at (0, 0, ...)

esdmI_hypercube_t* esdmI_hypercube_make(int64_t dimensions, int64_t* offset, int64_t* size);

esdmI_hypercube_t* esdmI_hypercube_makeCopy(esdmI_hypercube_t* original);

//returns NULL if the intersection is empty
esdmI_hypercube_t* esdmI_hypercube_makeIntersection(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

bool esdmI_hypercube_isEmpty(esdmI_hypercube_t* cube);

bool esdmI_hypercube_doesIntersect(esdmI_hypercube_t* a, esdmI_hypercube_t* b);

inline int64_t esdmI_hypercube_dimensions(esdmI_hypercube_t* cube) { return cube->dims; }

/**
 * Return the shape of the hypercube as an offset and a size vector.
 *
 * @param [in] cube the hypercube to get the shape of
 * @param [out] out_offset array of size cube->dims that will be filled with the offset vector components
 * @param [out] out_size array of size cube->dims that will be filled with the size vector components
 */
void esdmI_hypercube_getOffsetAndSize(esdmI_hypercube_t* cube, int64_t* out_offset, int64_t* out_size);

int64_t esdmI_hypercube_size(esdmI_hypercube_t* cube);

void esdmI_hypercube_print(esdmI_hypercube_t* cube, FILE* stream);

void esdmI_hypercube_destroy(esdmI_hypercube_t* cube);

esdmI_hypercubeSet_t* esdmI_hypercubeSet_make();  //convenience function to construct a heap allocated object
void esdmI_hypercubeSet_construct(esdmI_hypercubeSet_t* me);  //no allocation, initialization only

bool esdmI_hypercubeSet_isEmpty(esdmI_hypercubeSet_t* me);  //checks for logical emptiness, may remove empty hypercubes from the set

int64_t esdmI_hypercubeSet_count(esdmI_hypercubeSet_t* me);

void esdmI_hypercubeSet_add(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube);

void esdmI_hypercubeSet_subtract(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube);

bool esdmI_hypercubeSet_doesIntersect(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube);

bool esdmI_hypercubeSet_doesCoverFully(esdmI_hypercubeSet_t* me, esdmI_hypercube_t* cube);

void esdmI_hypercubeSet_print(esdmI_hypercubeSet_t* me, FILE* stream);  //for debugging purposes

void esdmI_hypercubeSet_destruct(esdmI_hypercubeSet_t* me); //counterpart to esdmI_hypercubeSet_construct()
void esdmI_hypercubeSet_destroy(esdmI_hypercubeSet_t* me);  //counterpart to esdmI_hypercubeSet_make()

#endif
