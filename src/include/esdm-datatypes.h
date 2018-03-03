/**
 * @file
 * @brief Datatype primitives provided by ESDM.
 */
#ifndef ESDM_DATATYPES_H
#define ESDM_DATATYPES_H


#include <stdint.h>
#include <glib.h>



typedef int esdm_type;

// ESDM Parameters and Status /////////////////////////////////////////////////
//
/**
 * ESDM Status codes and failure modes.
 */
typedef enum {
	ESDM_OVERWRITE,
	ESDM_CREATE,
	ESDM_AUTOCOMMIT,
	ESDM_DATASET,
	ESDM_CONTAINER,
} esdm_mode_t;

/**
 * ESDM Status codes and failure modes.
 */
typedef enum {
	ESDM_SUCCESS,
	ESDM_ERROR,
	ESDM_DIRTY,
	ESDM_PERSISTENT
} esdm_status_t;

/**
 * ESDM provides logging helpers, the available loglevels are defined here.
 */
typedef enum {
	ESDM_LOGLEVEL_CRITICAL,
	ESDM_LOGLEVEL_ERROR,
	ESDM_LOGLEVEL_WARNING,
	ESDM_LOGLEVEL_INFO,
	ESDM_LOGLEVEL_DEBUG,
	ESDM_LOGLEVEL_NOTSET
} esdm_loglevel_t;


// LOGICAL/DOMAIN DATATYPES ///////////////////////////////////////////////////

typedef struct esdm_container_t esdm_container_t;
typedef struct esdm_metadata_t esdm_metadata_t;
typedef struct esdm_dataset_t esdm_dataset_t;
typedef struct esdm_dataspace_t esdm_dataspace_t;
typedef struct esdm_fragment_t esdm_fragment_t;


struct esdm_container_t {
	esdm_metadata_t *metadata;	
	GHashTable *datasets;
	esdm_status_t status;
};

struct esdm_metadata_t {
	char *json;
};

struct esdm_dataset_t {
	esdm_metadata_t *metadata;
	esdm_dataspace_t *dataspace;
	GHashTable *fragments;
	esdm_status_t status;
};

struct esdm_dataspace_t {
	char *json;
	int type;
	int inspect_callback;
};

struct esdm_fragment_t {
	esdm_metadata_t *metadata;
	esdm_dataset_t *dataset;
	esdm_dataspace_t *dataspace;
	char *data;
	size_t size;
	esdm_status_t status;
};




typedef struct {
	char *json;
	GHashTable *fragments;
} esdm_fragment_index_t;


// MODULES ////////////////////////////////////////////////////////////////////

/**
 *	
 */
typedef enum {
	ESDM_TYPE_DATA,
	ESDM_TYPE_METADATA,
	ESDM_TYPE_HYBRID
} esdm_module_type_t;


typedef struct esdm_backend_t esdm_backend_t;
typedef struct esdm_backend_callbacks_t esdm_backend_callbacks_t;


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
struct esdm_backend_callbacks_t {
	int (*finalize)();
	int (*performance_estimate)(esdm_backend_t*);
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
};


/**
 * On backend registration ESDM expects the backend to return a pointer to 
 * a esdm_backend_t struct.
 *
 * Each backend provides
 *
 */
struct esdm_backend_t {
	char *name;
	esdm_module_type_t type;
	char *version; // 0.0.0
	void *data;
	uint32_t blocksize; /* any io must be multiple of 'blocksize' and aligned. */
	esdm_backend_callbacks_t callbacks;
};


///////////////////////////////////////////////////////////////////////////////
// INTERNAL
///////////////////////////////////////////////////////////////////////////////

// Organisation structures of core components /////////////////////////////////

// Config
typedef struct {
	const char *type;
	const char *name;
	const char *target;
} esdm_config_backend_t;

typedef struct {
	int count;
	esdm_config_backend_t *backends;
} esdm_config_backends_t;



// Modules
typedef struct {
	int count;
	esdm_module_type_t *module;
} esdm_module_type_array_t;


// Scheduler
typedef struct {
	int member;
	int callback;
} esdm_io_t;



// Performance Model
typedef struct {
	int latency;
	int throughout;
	int max_bytes;
	int min_bytes;
	int concurrency;
} esdm_performance_estimate_t;



// Entry points and state for core components /////////////////////////////////

typedef struct {
	void *json;
} esdm_config_t;

typedef struct {
	GHashTable *backends;
	esdm_backend_t* metadata;
	//esdm_modules_t** modules;
} esdm_modules_t;

typedef struct {
	int info;
	GHashTable *containers;
} esdm_layout_t;

typedef struct {
	int info;
} esdm_scheduler_t;

typedef struct {
	int info;
	GHashTable *cache;
} esdm_performance_t;


typedef struct {
	int is_initialized;
	esdm_config_t *config;
	esdm_modules_t *modules;
	esdm_layout_t *layout;
	esdm_scheduler_t *scheduler;
	esdm_performance_t *performance;
} esdm_instance_t;


#endif
