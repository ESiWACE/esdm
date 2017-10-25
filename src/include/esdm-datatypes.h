/**
 * @file
 * @brief Datatype primitives provided by ESDM.
 */
#ifndef ESDM_DATATYPES_H
#define ESDM_DATATYPES_H


#include <stdint.h>



typedef int esdm_metadata_t;
typedef int esdm_dataset_t;
typedef int esdm_fragment_t;
typedef int esdm_backend_t;



typedef enum {
	ESDM_SUCCESS,
	ESDM_ERROR
} esdm_status_t;







typedef enum {
  ESDM_TYPE_BACKEND,
  ESDM_TYPE_METADATA,
  ESDM_TYPE_LAST
} esdm_module_type_t;

typedef struct {
  int count;
  esdm_module_type_t * module;
} esdm_module_type_array_t;






typedef struct {
	int member;
} esdm_pending_io_t;



typedef struct{
	int thread_count;
	esdm_backend_t * backend;
	esdm_pending_io_t * io;
} esdm_pending_fragments_t;



#endif
