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


///////////////////////////////////////////////////////////////////////////////
// ESDM ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// These functions must be used before calling init:
esdm_status esdm_set_procs_per_node(int procs);
esdm_status esdm_set_total_procs(int procs);
esdm_status esdm_load_config_str(const char * str);

esdm_status esdm_init();
esdm_status esdm_finalize();

///////////////////////////////////////////////////////////////////////////////
// Public API: POSIX Legacy Compatibility /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// General Helpers
esdm_status esdm_sync();
esdm_status esdm_stat(char* desc, char* result);

// Object Manipulation
esdm_status esdm_open(char* desc, int mode);
esdm_status esdm_create(char* desc, int mode, esdm_container_t**, esdm_dataset_t**);
esdm_status esdm_close(void * buf);

esdm_status esdm_write(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t* subspace);
esdm_status esdm_read(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t* subspace);


///////////////////////////////////////////////////////////////////////////////
// Public API: Data Model Manipulators ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Container
esdm_container_t* esdm_container_create(const char *name);
esdm_container_t* esdm_container_retrieve(const char * name);
esdm_status esdm_container_commit(esdm_container_t *container);
esdm_status esdm_container_destroy(esdm_container_t *container);

// Datset
esdm_dataset_t* esdm_dataset_create(esdm_container_t *container, char * name, esdm_dataspace_t *dataspace);
esdm_dataset_t* esdm_dataset_retrieve(esdm_container_t *container, const char * name);
esdm_status esdm_dataset_commit(esdm_dataset_t *dataset);
esdm_status esdm_dataset_destroy(esdm_dataset_t *dataset);

// Dataspace
esdm_dataspace_t* esdm_dataspace_create(int64_t dimensions, int64_t *bounds, esdm_datatype type);
esdm_dataspace_t* esdm_dataspace_deserialize(void *serialized_dataspace);
esdm_dataspace_t* esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dimensions, int64_t *size, int64_t *offset);
esdm_status esdm_dataspace_destroy(esdm_dataspace_t *dataspace);
esdm_status esdm_dataspace_serialize(esdm_dataspace_t *dataspace, void **out);
uint64_t      esdm_dataspace_element_count(esdm_dataspace_t *dataspace);
uint64_t      esdm_dataspace_size(esdm_dataspace_t *dataspace);
void         esdm_dataspace_string_descriptor(char* out_str, esdm_dataspace_t *dataspace);

esdm_status esdm_dataspace_overlap_str(esdm_dataspace_t *parent, char delim, char * str_size, char * str_offset, esdm_dataspace_t ** out_space);

// Fragment
esdm_fragment_t* esdm_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, void *buf);
esdm_fragment_t* esdm_fragment_deserialize(void *serialized_fragment);
esdm_status esdm_fragment_retrieve(esdm_fragment_t *fragment);
esdm_status esdm_fragment_commit(esdm_fragment_t *fragment);
esdm_status esdm_fragment_destroy(esdm_fragment_t *fragment);
esdm_status esdm_fragment_serialize(esdm_fragment_t *fragment, void **out);

void esdm_fragment_print(esdm_fragment_t *fragment);
void esdm_dataspace_print(esdm_dataspace_t *dataspace);

size_t esdm_sizeof(esdm_datatype type);

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
void print_stat(struct stat sb);

void mkdir_recursive(const char *path);
void posix_recursive_remove(const char * path);

int read_file(char *filepath, char **buf);

int read_check(int fd, char *buf, size_t len);
int write_check(int fd, char *buf, size_t len);


#endif
