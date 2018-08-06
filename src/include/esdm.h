/**
 * @file
 * @brief Public API of the ESDM. Inlcudes several other public interfaces.
 */
#ifndef ESDM_H
#define ESDM_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <jansson.h>

#include <esdm-datatypes.h>


///////////////////////////////////////////////////////////////////////////////
// ESDM ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// These functions must be used before calling init:
esdm_status_t esdm_set_procs_per_node(int procs);
esdm_status_t esdm_set_total_procs(int procs);
esdm_status_t esdm_load_config_str(const char * str);

esdm_status_t esdm_init();
esdm_status_t esdm_finalize();

///////////////////////////////////////////////////////////////////////////////
// Public API: POSIX Legacy Compatibility /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// General Helpers
esdm_status_t esdm_sync();
esdm_status_t esdm_stat(char* desc, char* result);

// Object Manipulation
esdm_status_t esdm_open(char* desc, int mode);
esdm_status_t esdm_create(char* desc, int mode, esdm_container_t**, esdm_dataset_t**);
esdm_status_t esdm_close(void * buf);

esdm_status_t esdm_write(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t* subspace);
esdm_status_t esdm_read(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t* subspace);


///////////////////////////////////////////////////////////////////////////////
// Public API: Data Model Manipulators ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Container
esdm_container_t* esdm_container_create(const char *name);
esdm_container_t* esdm_container_retrieve(const char * name);
esdm_status_t esdm_container_commit(esdm_container_t *container);
esdm_status_t esdm_container_destroy(esdm_container_t *container);

// Datset
esdm_dataset_t* esdm_dataset_create(esdm_container_t *container, char * name, esdm_dataspace_t *dataspace);
esdm_dataset_t* esdm_dataset_retrieve(esdm_container_t *container, const char * name);
esdm_status_t esdm_dataset_commit(esdm_dataset_t *dataset);
esdm_status_t esdm_dataset_destroy(esdm_dataset_t *dataset);

// Dataspace
esdm_dataspace_t* esdm_dataspace_create(int64_t dimensions, int64_t *bounds, esdm_datatype_t type);
esdm_dataspace_t* esdm_dataspace_deserialize(void *serialized_dataspace);
esdm_dataspace_t* esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dimensions, int64_t *size, int64_t *offset);
esdm_status_t esdm_dataspace_destroy(esdm_dataspace_t *dataspace);
esdm_status_t esdm_dataspace_serialize(esdm_dataspace_t *dataspace, void **out);
uint64_t      esdm_dataspace_element_count(esdm_dataspace_t *dataspace);
uint64_t      esdm_dataspace_size(esdm_dataspace_t *dataspace);
void         esdm_dataspace_string_descriptor(char* out_str, esdm_dataspace_t *dataspace);

esdm_status_t esdm_dataspace_overlap_str(esdm_dataspace_t *parent, char delim, char * str_size, char * str_offset, esdm_dataspace_t ** out_space);

/*
 * This function computes the offset to a starting buffer relative to the dataspace
 */
uint64_t      esdm_buffer_offset_first_dimension(esdm_dataspace_t *dataspace, int64_t offset);

// Fragment
esdm_fragment_t* esdm_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, void *buf);
esdm_fragment_t* esdm_fragment_deserialize(void *serialized_fragment);
esdm_status_t esdm_fragment_retrieve(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_commit(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_destroy(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_serialize(esdm_fragment_t *fragment, void **out);

void esdm_fragment_print(esdm_fragment_t *fragment);
void esdm_dataspace_print(esdm_dataspace_t *dataspace);

size_t esdm_sizeof(esdm_datatype_t type);




///////////////////////////////////////////////////////////////////////////////
// UTILS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// json.c /////////////////////////////////////////////////////////////////////
// load json helper
json_t *load_json(const char *text);

// print json helper
void print_json(const json_t *root);
void print_json_aux(const json_t *element, int indent);
const char *json_plural(int count);

// json path for convienient access
int json_path_set(json_t *json, const char *path, json_t *value, unsigned int append);
json_t *json_path_get(const json_t *json, const char *path);

// auxiliary.c ////////////////////////////////////////////////////////////////
void print_stat(struct stat sb);
int read_file(char *filepath, char **buf);
void mkdir_recursive(const char *path);


#endif
