#ifndef ESDM_DATATYPES_INTERNAL_H
#define ESDM_DATATYPES_INTERNAL_H

#include <esdm-datatypes.h>

#include <glib.h>
#include <jansson.h>


struct esdm_container_t_t {
  char *name;
  esdm_metadata_t *metadata;
  GHashTable *datasets;
  esdm_status status;
};

struct esdm_json_t {
  char *json;
  int buff_size;
  int size;
};

struct esdm_metadata_t {
  char *json;
  int buff_size;
  int size;
  smd_attr_t *attr;
};


struct esdm_dataset_t {
  char *name;
  esdm_container_t *container;
  esdm_metadata_t *metadata;
  esdm_dataspace_t *dataspace;
  GHashTable *fragments;
  esdm_status status;
};


struct esdm_fragment_t {
  esdm_metadata_t *metadata; // only valid after written
  esdm_dataset_t *dataset;
  esdm_dataspace_t *dataspace;
  esdm_backend_t *backend;
  void *buf;
  int in_place;
  size_t elements;
  size_t bytes;
  esdm_status status;
};


// multiple fragments
typedef struct {
  struct esdm_fragment_t *fragment;
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


typedef struct {
  int x;
} esdm_dataset_iterator_t;


// MODULES ////////////////////////////////////////////////////////////////////

/**
 * ESDM Module Types
 *
 *     DATA		   A backend that handles data
 *     METADATA    A backend that handles metadata
 *     HYBRID      A backend that handles both
 *
 */
typedef enum esdm_module_type_t {
  SMD_DTYPE_DATA,
  SMD_DTYPE_METADATA,
  SMD_DTYPE_HYBRID
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
struct esdm_backend_t_callbacks_t {
  // General for ESDM
  int (*finalize)(esdm_backend_t *);
  int (*performance_estimate)(esdm_backend_t *, esdm_fragment_t *fragment, float *out_time);

  // Data Callbacks (POSIX like)
  int (*create)(esdm_backend_t *, char *name);
  int (*open)(esdm_backend_t *);
  int (*write)(esdm_backend_t *);
  int (*read)(esdm_backend_t *);
  int (*close)(esdm_backend_t *);

  // Metadata Callbacks
  /*
	 * Retrieve a list (and metadata) of fragments that contain data for the given subpatch with the size and offset.
	 */
  int (*lookup)(esdm_backend_t *b, esdm_dataset_t *dataset, esdm_dataspace_t *space, int *out_frag_count, esdm_fragment_t ***out_fragments);

  // ESDM Data Model Specific
  int (*container_create)(esdm_backend_t *, esdm_container_t *container);
  int (*container_retrieve)(esdm_backend_t *, esdm_container_t *container);
  int (*container_update)(esdm_backend_t *, esdm_container_t *container);
  int (*container_destroy)(esdm_backend_t *, esdm_container_t *container);

  int (*dataset_create)(esdm_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_retrieve)(esdm_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_update)(esdm_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_destroy)(esdm_backend_t *, esdm_dataset_t *dataset);

  int (*fragment_create)(esdm_backend_t *, esdm_fragment_t *fragment);
  int (*fragment_retrieve)(esdm_backend_t *, esdm_fragment_t *fragment, json_t *metadata);
  int (*fragment_update)(esdm_backend_t *, esdm_fragment_t *fragment);
  int (*fragment_destroy)(esdm_backend_t *, esdm_fragment_t *fragment);

  int (*mkfs)(esdm_backend_t *, int enforce_format);
};

struct esdm_md_backend_callbacks_t {
  // General for ESDM
  int (*finalize)(esdm_md_backend_t *);
  int (*performance_estimate)(esdm_md_backend_t *, esdm_fragment_t *fragment, float *out_time);

  // Metadata Callbacks
  /*
	 * Retrieve a list (and metadata) of fragments that contain data for the given subpatch with the size and offset.
	 */
  int (*lookup)(esdm_md_backend_t *b, esdm_dataset_t *dataset, esdm_dataspace_t *space, int *out_frag_count, esdm_fragment_t ***out_fragments);

  // ESDM Data Model Specific
  int (*container_create)(esdm_md_backend_t *, esdm_container_t *container);
  int (*container_retrieve)(esdm_md_backend_t *, esdm_container_t *container);
  int (*container_update)(esdm_md_backend_t *, esdm_container_t *container);
  int (*container_destroy)(esdm_md_backend_t *, esdm_container_t *container);

  int (*dataset_create)(esdm_md_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_retrieve)(esdm_md_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_update)(esdm_md_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_destroy)(esdm_md_backend_t *, esdm_dataset_t *dataset);

  int (*fragment_create)(esdm_md_backend_t *, esdm_fragment_t *fragment);
  int (*fragment_retrieve)(esdm_md_backend_t *, esdm_fragment_t *fragment, json_t *metadata);
  int (*fragment_update)(esdm_md_backend_t *, esdm_fragment_t *fragment);
  int (*fragment_destroy)(esdm_md_backend_t *, esdm_fragment_t *fragment);

  int (*mkfs)(esdm_md_backend_t *, int enforce_format);
};


/**
 * On backend registration ESDM expects the backend to return a pointer to
 * a esdm_backend_t struct.
 *
 * Each backend provides
 *
 */
struct esdm_backend_t {
  esdm_config_backend_t *config;
  char *name;
  esdm_module_type_t type;
  char *version; // 0.0.0
  void *data;
  uint32_t blocksize; /* any io must be multiple of 'blocksize' and aligned. */
  esdm_backend_t_callbacks_t callbacks;
  int threads;
  GThreadPool *threadPool;
};

struct esdm_md_backend_t {
  esdm_config_backend_t *config;
  char *name;
  char *version;
  void *data;
  esdm_md_backend_callbacks_t callbacks;
};

typedef enum io_operation_t {
  ESDM_OP_WRITE = 0,
  ESDM_OP_READ
} io_operation_t;

typedef struct io_request_status_t {
  int pending_ops;
  GMutex mutex;
  GCond done_condition;
} io_request_status_t;

typedef struct {
  void *mem_buf;
  esdm_dataspace_t *buf_space;
} io_work_callback_data_t;

typedef struct io_work_t io_work_t;

struct io_work_t {
  esdm_fragment_t *fragment;
  io_operation_t op;
  esdm_status return_code;
  io_request_status_t *parent;
  void (*callback)(io_work_t *work);
  io_work_callback_data_t data;
};


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


struct esdm_config_backends_t {
  int count;
  esdm_config_backend_t *backends;
};


typedef struct esdm_config_backends_t esdm_config_backends_t;


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
  int data_backend_count;
  esdm_backend_t **data_backends;
  esdm_md_backend_t *metadata_backend;
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


typedef struct esdm_instance_t esdm_instance_t;


struct esdm_instance_t {
  int is_initialized;
  int procs_per_node;
  int total_procs;
  esdm_config_t *config;
  esdm_modules_t *modules;
  esdm_layout_t *layout;
  esdm_scheduler_t *scheduler;
  esdm_performance_t *performance;
};


// Auxiliary? /////////////////////////////////////////////////////////////////

typedef struct esdm_bytesequence_t esdm_bytesyquence_t;
struct esdm_bytesequence {
  esdm_datatype_t type;
  size_t count;
  void *data;
};


#endif
