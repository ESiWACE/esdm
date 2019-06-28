/**
 * @file
 * @brief Public API of the ESDM. Inlcudes several other public interfaces.
 */
#ifndef ESDM_H
#define ESDM_H

#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <jansson.h>

#include <esdm-datatypes.h>
#include <esdm-internal.h>
#include <esdm-datatypes-internal.h>


///////////////////////////////////////////////////////////////////////////////
// ESDM ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// These functions must be used before calling init:
esdm_status esdm_set_procs_per_node(int procs);
esdm_status esdm_set_total_procs(int procs);
esdm_status esdm_load_config_str(const char * str);

/**
 * Initialize ESDM:
 *	- allocate data structures for ESDM
 *	- allocate memory for node local caches
 *	- initialize submodules
 *	- initialize threadpool
 *
 * @return status
 */

esdm_status esdm_init();

/**
 * Display status information for objects stored in ESDM.
 *
 * @param [in] desc	Name or descriptor of object.
 *
 * @return Status
 */

esdm_status esdm_finalize();

///////////////////////////////////////////////////////////////////////////////
// Public API: POSIX Legacy Compatibility /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// General Helpers

/**
 * Ensure all remaining data is syncronized with backends.
 * If not called at the end of an application, ESDM can not guarantee all data
 * was written.
 *
 * @return Status
 */

esdm_status esdm_sync();

/**
 * Display status information for objects stored in ESDM.
 *
 * @param [in]	desc	name or descriptor of object
 * @param [out]	result	where to write result of query
 *
 * @return Status
 */

esdm_status esdm_stat(char* desc, char* result);

// Object Manipulation

/**
 * Open a existing object.
 *
 * TODO: decide if also useable to create?
 *
 * @param [in] desc		string object identifier
 * @param [in] mode		mode flags for open/creation
 *
 * @return Status
 */

esdm_status esdm_open(char* desc, int mode);

/**
 * Create a new object.
 *
 * @param [in]	desc		string object identifier
 * @param [in]	mode		mode flags for creation
 * @param [out] container	pointer to new container
 *
 * @return Status
 */

esdm_status esdm_create(char* desc, int mode, esdm_container**, esdm_dataset_t**);

/**
 * Close opened object.
 *
 * @param [in] desc		String Object Identifier
 *
 * @return Status
 */

esdm_status esdm_close(void * buf);

/**
 * Write data  of size starting from offset.
 *
 * @param [in] buf	The pointer to a contiguous memory region that shall be written
 * @param [in] dset	TODO, currently a stub, we assume it has been identified/created before...., json description?
 * @param [in] dims	The number of dimensions, needed for size and offset
 * @param [in] size	...
 *
 * @return Status
 */

esdm_status esdm_write(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t* subspace);

/**
 * Reads a data fragment described by desc to the dataset dset.
 *
 * @param [out] buf	The pointer to a contiguous memory region that shall be written
 * @param [in] dset	TODO, currently a stub, we assume it has been identified/created before.... , json description?
 * @param [in] dims	The number of dimensions, needed for size and offset
 * @param [in] size	...
 *
 * @return Status
 */

esdm_status esdm_read(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t* subspace);

///////////////////////////////////////////////////////////////////////////////
// Public API: Data Model Manipulators ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Container //////////////////////////////////////////////////////////////////

/**
 * Create a new container.
 *
 *  - Allocate process local memory structures.
 *	- Register with metadata service.
 *
 *	@return Pointer to new container.
 *
 */

esdm_status esdm_container_create(const char* name, esdm_container **out_container);
esdm_status esdm_container_retrieve(const char * name, esdm_container **out_container);

/**
 * Make container persistent to storage.
 * Enqueue for writing to backends.
 *
 * Calling container commit may trigger subsequent commits for datasets that
 * are part of the container.
 *
 */

esdm_status esdm_container_commit(esdm_container *container);

/**
 * Destroy a existing container.
 *
 * Either from memory or from persistent storage.
 *
 */

esdm_status esdm_container_destroy(esdm_container *container);

// Dataset

/**
 * Create a new dataset.
 *
 *  - Allocate process local memory structures.
 *	- Register with metadata service.
 *
 *	@return Pointer to new dateset.
 *
 */

esdm_status esdm_dataset_create(esdm_container* container, const char* name, esdm_dataspace_t* dataspace, esdm_dataset_t ** out_dataset);
esdm_status esdm_dataset_name_dimensions(esdm_dataset_t * dataset, int dims, char ** names);
esdm_status esdm_dataset_get_dataspace(esdm_dataset_t *dset, esdm_dataspace_t ** out_dataspace);

esdm_status esdm_dataset_iterator(esdm_container *container, esdm_dataset_iterator_t ** out_iter);
esdm_status esdm_dataset_retrieve(esdm_container *container, const char * name, esdm_dataset_t **out_dataset);

/**
 * Make dataset persistent to storage.
 * Schedule for writing to backends.
 */

esdm_status esdm_dataset_commit(esdm_dataset_t *dataset);
esdm_status esdm_dataset_destroy(esdm_dataset_t *dataset);
/* This function adds the metadata to the ESDM */
esdm_status esdm_dataset_link_attribute (esdm_dataset_t * dset, smd_attr_t * attr);

/* This function returns the attributes */
esdm_status esdm_dataset_get_attributes (esdm_dataset_t *dataset, smd_attr_t ** out_metadata);


// Dataspace

/**
 * Create a new dataspace.
 *
 *  - Allocate process local memory structures.
 *
 *	@return Pointer to new dateset.
 *
 */

esdm_status esdm_dataspace_create(int64_t dimensions, int64_t* sizes, esdm_datatype_t datatype, esdm_dataspace_t ** out_dataspace);

/**
 * Reinstantiate dataspace from serialization.
 */

esdm_status esdm_dataspace_deserialize(void *serialized_dataspace, esdm_dataspace_t ** out_dataspace);
esdm_status esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dimensions, int64_t *size, int64_t *offset, esdm_dataspace_t **out_dataspace);

/**
 * Destroy dataspace in memory.
 */

esdm_status esdm_dataspace_destroy(esdm_dataspace_t *dataspace);

/**
 * Serializes dataspace description.
 *
 * e.g., to store along with fragment
 */

esdm_status esdm_dataspace_serialize(esdm_dataspace_t *dataspace, void **out);
uint64_t    esdm_dataspace_element_count(esdm_dataspace_t *dataspace);
uint64_t    esdm_dataspace_size(esdm_dataspace_t *dataspace);
void        esdm_dataspace_string_descriptor(char* out_str, esdm_dataspace_t *dataspace);

esdm_status esdm_dataspace_overlap_str(esdm_dataspace_t *parent, char delim, char * str_size, char * str_offset, esdm_dataspace_t ** out_space);

// Fragment

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

esdm_status esdm_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, void *buf, esdm_fragment_t ** out_fragment);

/**
 * Reinstantiate fragment from serialization.
 */

esdm_status esdm_fragment_deserialize(void *serialized_fragment, esdm_fragment_t ** _out_fragment);
esdm_status esdm_fragment_retrieve(esdm_fragment_t *fragment);

/**
 * Make fragment persistent to storage.
 * Schedule for writing to backends.
 */

esdm_status esdm_fragment_commit(esdm_fragment_t *fragment);
esdm_status esdm_fragment_destroy(esdm_fragment_t *fragment);

/**
 * Serializes fragment for storage.
 *
 * @startuml{fragment_serialization.png}
 *
 * User -> Fragment: serialize()
 *
 * Fragment -> Dataspace: serialize()
 * Fragment <- Dataspace: (status, string)
 *
 * User <- Fragment: (status, string)
 *
 * @enduml
 *
 */

esdm_status esdm_fragment_serialize(esdm_fragment_t *fragment, void **out);

void esdm_fragment_print(esdm_fragment_t *fragment);
void esdm_dataspace_print(esdm_dataspace_t *dataspace);


//size_t esdm_sizeof(esdm_datatype_t type);
#define esdm_sizeof(type) (type->size)

 /**
  * Initialize backend by invoking mkfs callback for matching target
  *
  * @param [in] enforce_format  force reformatting existing system (may result in data loss)
  * @param [in] target  target descriptor
  *
  * @return Status
  */

  /*
   * enforce_format = 1 => recreate structure deleting old stuff
   * enforce_format = 2 => delete only
   */

esdm_status esdm_mkfs(int enforce_format, data_accessibility_t target);


///////////////////////////////////////////////////////////////////////////////
// UTILS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// json.c /////////////////////////////////////////////////////////////////////
// load json helper

/*
 * Parse text into a JSON object. If text is valid JSON, returns a
 * json_t structure, otherwise prints and error and returns null.
 */

json_t *load_json(const char *text);

// print json helper
void print_json(const json_t *root);
void print_json_aux(const json_t *element, int indent);
const char *json_plural(int count);

// json path for convienient access
int json_path_set_new(json_t *json, const char *path, json_t *value, size_t flags, json_error_t *error);
json_t *json_path_get(const json_t *json, const char *path);

static inline
int json_path_set(json_t *json, const char *path, json_t *value, size_t flags, json_error_t *error)
{
    return json_path_set_new(json, path, json_incref(value), flags, error);
}

// auxiliary.c ////////////////////////////////////////////////////////////////

/**
 * Print a detailed summary for the stat system call.
 */

void print_stat(struct stat sb);

int mkdir_recursive(const char *path);
void posix_recursive_remove(const char * path);

int read_file(char *filepath, char **buf);

/**
 * Read while ensuring and retrying until len is read or error occured.
 */

int read_check(int fd, char *buf, size_t len);

/**
 * Write while ensuring and retrying until len is written or error occured.
 */

int write_check(int fd, char *buf, size_t len);


#endif
