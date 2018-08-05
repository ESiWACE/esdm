/**
 * @file
 * @brief Datatype primitives provided by ESDM.
 */
#ifndef ESDM_DATATYPES_H
#define ESDM_DATATYPES_H


#include <stdint.h>
#include <glib.h>

#define ESDM_MAX_SIZE 1024

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


/*
Fixed width integer types

int8_t
uint8_t
int16_t
uint16_t
int32_t
uint32_t
int64_t
uint64_t

float			if IEEE 754 (32bit)
double			if IEEE 754 (64bit)
*/

typedef enum esdm_datatype_t {
	ESDM_TYPE_INT8_T,
	ESDM_TYPE_INT16_T,
	ESDM_TYPE_INT32_T,
	ESDM_TYPE_INT64_T,

	ESDM_TYPE_UINT8_T,
	ESDM_TYPE_UINT16_T,
	ESDM_TYPE_UINT32_T,
	ESDM_TYPE_UINT64_T,

	ESDM_TYPE_FLOAT,			// if IEEE 754 (32bit)
	ESDM_TYPE_DOUBLE,		// if IEEE 754 (64bit)

	ESDM_TYPE_CHAR,			// 1 byte
	ESDM_TYPE_CHAR_ASCII,	// 1 byte
	ESDM_TYPE_CHAR_UTF8,		// esdm_sizeof will fail here
	ESDM_TYPE_CHAR_UTF16,
	ESDM_TYPE_CHAR_UTF32,
} esdm_datatype_t;





/**
 * ESDM Status codes and failure modes.
 */
typedef enum {
	ESDM_SUCCESS = 0,
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
typedef struct esdm_backend_t esdm_backend_t;
typedef struct esdm_backend_callbacks_t esdm_backend_callbacks_t;


struct esdm_container_t {
	char *name;
	esdm_metadata_t *metadata;
	GHashTable *datasets;
	esdm_status_t status;
};

struct esdm_metadata_t {
	char *json;
	int size;
};

struct esdm_dataset_t {
	char *name;
	esdm_container_t *container;
	esdm_metadata_t *metadata;
	esdm_dataspace_t *dataspace;
	GHashTable *fragments;
	esdm_status_t status;
};

struct esdm_dataspace_t {
	esdm_datatype_t datatype;
	int64_t dimensions;
	int64_t *size;

	esdm_dataspace_t *subspace_of;
	int64_t *offset;

	char *json;
};



struct esdm_fragment_t {
	esdm_metadata_t  *metadata; // only valid after written
	esdm_dataset_t 	 *dataset;
	esdm_dataspace_t *dataspace;

	esdm_backend_t * backend;

	void *buf;
	size_t elements;
	size_t bytes;
	esdm_status_t status;
};

// multiple fragments
typedef struct{
	struct esdm_fragment_t * fragment;
	int count;
} esdm_fragments_t;

typedef struct esdm_fragment_index_t {
	char *json;
	GHashTable *fragments;
	/*
	int (callback_insert)();
	int (callback_remove)();
	int (callback_lookup)();
	*/
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
// General for ESDM
	int (*finalize)(esdm_backend_t*);
	int (*performance_estimate)(esdm_backend_t*, esdm_fragment_t *fragment, float * out_time);

// Data Callbacks (POSIX like)
	int (*create)(esdm_backend_t*, char *name);
	int (*open)(esdm_backend_t*);
	int (*write)(esdm_backend_t*);
	int (*read)(esdm_backend_t*);
	int (*close)(esdm_backend_t*);

// Metadata Callbacks
  /*
	 * Retrieve a list (and metadata) of fragments that contain data for the given subpatch with the size and offset.
	 */
	int (*lookup)(esdm_backend_t * b, esdm_dataset_t * dataset,	esdm_dataspace_t * space, int * out_frag_count, esdm_fragment_t *** out_fragments);

// ESDM Data Model Specific
	int (*container_create)(esdm_backend_t*, esdm_container_t *container);
	int (*container_retrieve)(esdm_backend_t*, esdm_container_t *container);
	int (*container_update)(esdm_backend_t*, esdm_container_t *container);
	int (*container_destroy)(esdm_backend_t*, esdm_container_t *container);

	int (*dataset_create)(esdm_backend_t*, esdm_dataset_t *dataset);
	int (*dataset_retrieve)(esdm_backend_t*, esdm_dataset_t *dataset);
	int (*dataset_update)(esdm_backend_t*, esdm_dataset_t *dataset);
	int (*dataset_destroy)(esdm_backend_t*, esdm_dataset_t *dataset);

	int (*fragment_create)(esdm_backend_t*, esdm_fragment_t *fragment);
	int (*fragment_retrieve)(esdm_backend_t*, esdm_fragment_t *fragment, json_t *metadata);
	int (*fragment_update)(esdm_backend_t*, esdm_fragment_t *fragment);
	int (*fragment_destroy)(esdm_backend_t*, esdm_fragment_t *fragment);
};


typedef struct esdm_config_backend_t esdm_config_backend_t;

/**
 * On backend registration ESDM expects the backend to return a pointer to
 * a esdm_backend_t struct.
 *
 * Each backend provides
 *
 */
struct esdm_backend_t {
	esdm_config_backend_t * config;
	char *name;
	esdm_module_type_t type;
	char *version; // 0.0.0
	void *data;
	uint32_t blocksize; /* any io must be multiple of 'blocksize' and aligned. */
	esdm_backend_callbacks_t callbacks;
	int threads;

	GThreadPool * threadPool;
};

typedef enum {
	ESDM_OP_WRITE = 0,
	ESDM_OP_READ
} io_operation_t;

typedef struct{
  int pending_ops;
  GMutex mutex;
  GCond  done_condition;
} io_request_status_t;

typedef struct{
  esdm_fragment_t *fragment;
	io_operation_t op;
	esdm_status_t return_code;

  io_request_status_t * parent;
} io_work_t;

// where is the data accessible
typedef enum {
	ESDM_ACCESSIBILITY_GLOBAL, // shared file system etc.
	ESDM_ACCESSIBILITY_NODELOCAL
} data_accessibility_t;


///////////////////////////////////////////////////////////////////////////////
// INTERNAL
///////////////////////////////////////////////////////////////////////////////

// Organisation structures of core components /////////////////////////////////


// Configuration
struct esdm_config_backend_t {
	const char *type;
	const char *id;
	const char *target;

	int max_threads_per_node;
	int max_global_threads;
	uint64_t max_fragment_size;
	data_accessibility_t data_accessibility;

	json_t *performance_model;
	json_t *esdm;
	json_t *backend;
};


typedef struct esdm_config_backends_t esdm_config_backends_t;
struct esdm_config_backends_t {
	int count;
	esdm_config_backend_t *backends;
};


// Modules
typedef struct esdm_module_type_array_t esdm_module_type_array_t;
struct esdm_module_type_array_t {
	int count;
	esdm_module_type_t *module;
};


// Scheduler
typedef struct esdm_io_t esdm_io_t;
struct esdm_io_t {
	int member;
	int callback;
};


// Entry points and state for core components /////////////////////////////////

typedef struct esdm_config_t {
	void *json;
} esdm_config_t;

typedef struct esdm_modules_t {
	int backend_count;
	esdm_backend_t **backends;
	esdm_backend_t *metadata;
	//esdm_modules_t** modules;
} esdm_modules_t;

typedef struct esdm_layout_t {
	int info;
	GHashTable *containers;
} esdm_layout_t;

typedef struct esdm_scheduler_t {
	int info;
	GThreadPool *thread_pool;
	GAsyncQueue *read_queue;
	GAsyncQueue *write_queue;
} esdm_scheduler_t;

typedef struct esdm_performance_t {
	int info;
	GHashTable *cache;
} esdm_performance_t;


typedef struct esdm_instance_t {
	int is_initialized;
	int procs_per_node;
	int total_procs;
	esdm_config_t *config;
	esdm_modules_t *modules;
	esdm_layout_t *layout;
	esdm_scheduler_t *scheduler;
	esdm_performance_t *performance;
} esdm_instance_t;







// Auxiliary? /////////////////////////////////////////////////////////////////

typedef struct esdm_bytesequence_t esdm_bytesyquence_t;
struct esdm_bytesequence {
	esdm_datatype_t type;
	size_t count;
	void * data;
};




#endif
