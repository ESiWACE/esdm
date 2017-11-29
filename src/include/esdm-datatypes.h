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





/**
 * ESDM Status codes and failure modes.
 *
 *
 *
 *
 */
typedef enum {
	ESDM_SUCCESS,
	ESDM_ERROR
} esdm_status_t;


typedef enum {
	ESDM_TYPE_DATA,
	ESDM_TYPE_METADATA,
	ESDM_TYPE_LAST
} esdm_module_type_t;



// Callbacks
typedef struct {
	int (*finalize)();
	int (*performance_estimate)();
//} esdm_backend_callbacks_esdm_t;
//
// Data Callbacks
//typedef struct {
	int (*create)();
	int (*open)();
	int (*write)();
	int (*read)();
	int (*close)();
//} esdm_backend_callbacks_data_t;
//
// Metadata Callbacks
//typedef struct {
	int (*allocate)();
	int (*update)();
	int (*lookup)();
//} esdm_module_callbacks_metadata_t;
} esdm_backend_callbacks_t;








typedef struct {
	char* name;
	esdm_module_type_t type;
	esdm_backend_callbacks_t callbacks;
} esdm_backend_t;







// INTERNAL
///////////////////////////////////////////////////////////////////////////////

// Module Management
typedef struct {
	int count;
	esdm_module_type_t * module;
} esdm_module_type_array_t;


// Layout
typedef struct {
	int member;
} esdm_pending_fragment_t;






// Scheduler
typedef struct{
	int thread_count;
	esdm_backend_t * backend;
	esdm_pending_fragment_t * io;
} esdm_pending_fragments_t;


// Performance Model
typedef struct {
	int latency;
	int throughout;
	int max_bytes;
	int min_bytes;
} esdm_performance_estimate;


#endif
