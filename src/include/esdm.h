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
// Application facing API /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// General Helpers
esdm_status_t esdm_sync();
esdm_status_t esdm_stat(char* desc, char* result);

// Object Manipulation
esdm_status_t esdm_open(char* desc, int mode);
esdm_status_t esdm_create(char* desc, int mode);
esdm_status_t esdm_close(void * buf);

esdm_status_t esdm_write(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset);
esdm_status_t esdm_read(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset);


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
