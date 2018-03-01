/**
 * @file
 * @brief Public API of the ESDM. Inlcudes several other public interfaces.
 */
#ifndef ESDM_H
#define ESDM_H

#include <stddef.h>

#include <jansson.h>

#include <esdm-datatypes.h>


///////////////////////////////////////////////////////////////////////////////
// ESDM ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdm_status_t esdm_init();
esdm_status_t esdm_finalize();


///////////////////////////////////////////////////////////////////////////////
// Public API: POSIX Legacy Compaitbility /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// General Helpers
esdm_status_t esdm_sync();
esdm_status_t esdm_stat(char* desc, char* result);

// Object Manipulation
esdm_status_t esdm_open(char* desc, int mode);
esdm_status_t esdm_create(char* desc, int mode, esdm_container_t**);
esdm_status_t esdm_close(void * buf);

esdm_status_t esdm_write(esdm_container_t *container, void *buf, int dims, uint64_t * size, uint64_t* offset);
esdm_status_t esdm_read(esdm_container_t *container, void *buf, int dims, uint64_t * size, uint64_t* offset);


///////////////////////////////////////////////////////////////////////////////
// Public API: Data Model Manipulators ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Container
esdm_container_t* esdm_container_create(const char *name);
esdm_status_t esdm_container_commit(esdm_container_t *container);
esdm_status_t esdm_container_destroy(esdm_container_t *container);

// Datset
esdm_dataset_t* esdm_dataset_create(esdm_container_t* container, char * name, esdm_dataspace_t* dataspace);
esdm_status_t esdm_dataset_commit(esdm_dataset_t *dataset);
esdm_status_t esdm_dataset_destroy(esdm_dataset_t *dataset);

// Dataspace
esdm_dataspace_t* esdm_dataspace_create();
esdm_status_t esdm_dataspace_destroy(esdm_dataspace_t *dataspace);
esdm_status_t esdm_dataspace_serialize(esdm_dataspace_t *dataspace, char **out);
esdm_dataspace_t* esdm_dataspace_deserialize(char *serialized_dataspace);

// Fragment
esdm_fragment_t* esdm_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, char *data);
esdm_status_t esdm_fragment_commit(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_destroy(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_serialize(esdm_fragment_t *fragment, char **out);
esdm_fragment_t* esdm_fragment_deserialize(char *serialized_fragment);




///////////////////////////////////////////////////////////////////////////////
// UTILS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// load json helper
json_t *load_json(const char *text);

// print json helper
void print_json(const json_t *root);
void print_json_aux(const json_t *element, int indent);
const char *json_plural(int count);

// json path for convienient access
int json_path_set(json_t *json, const char *path, json_t *value, unsigned int append);
json_t *json_path_get(const json_t *json, const char *path);





#endif
