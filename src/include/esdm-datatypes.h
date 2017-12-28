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





/**
 * ESDM provides logging helpers, the available loglevels are defined here.
 *
 */
typedef enum {
	ESDM_LOGLEVEL_CRITICAL,
	ESDM_LOGLEVEL_ERROR,
	ESDM_LOGLEVEL_WARNING,
	ESDM_LOGLEVEL_INFO,
	ESDM_LOGLEVEL_DEBUG,
	ESDM_LOGLEVEL_NOTSET
} esdm_loglevel_t;





// Callbacks
/**
 *
 * finalize:
 *		before ESDM exits, it will call the finalize function for every module
 *
 * performance_estimate:
 *		
 *	
 *
 * 
 * Notes:
 *	* No callback is expected for initialization as ESDM calls it on discovery.
 *
 */
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







/**
 * On backend registration ESDM expects the backend to return a pointer to 
 * a esdm_backend_t struct.
 *
 * Each backend provides
 *
 */
typedef struct {
	char* name;
	esdm_module_type_t type;
	char* version; // 0.0.0
	void* data;
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




// Scheduler
typedef struct {
	int member;
} esdm_pending_fragment_t;


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
	int concurrency;
} esdm_performance_estimate_t;


#endif
