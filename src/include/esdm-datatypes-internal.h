#ifndef ESDM_DATATYPES_INTERNAL_H
#define ESDM_DATATYPES_INTERNAL_H

#include <jansson.h>
#include <glib.h>
#include <stdatomic.h>

#include <esdm-datatypes.h>
#include <smd-datatype.h>

enum esdm_data_status_e {
  ESDM_DATA_NOT_LOADED,
  ESDM_DATA_LOADING,
  ESDM_DATA_DIRTY,
  ESDM_DATA_PERSISTENT,
  ESDM_DATA_DELETED
};

typedef enum esdm_data_status_e esdm_data_status_e;

struct esdm_datasets_t {
  esdm_dataset_t ** dset;
  int count;
  int buff_size;
};

struct esdm_container_t {
  char *name;
  smd_attr_t *attr;
  esdm_datasets_t dsets;

  int refcount;
  esdm_data_status_e status;
  int mode_flags; // set via esdm_mode_flags_e
};

struct esdm_fragments_t {
  esdm_fragment_t ** frag;
  int count;
  int buff_size;
};

typedef struct esdm_fragments_t esdm_fragments_t;

struct esdm_dataset_t {
  char *name;
  char *id;
  char **dims_dset_id; // array of variable names != NULL if set
  esdm_container_t *container;
  esdm_dataspace_t *dataspace;
  smd_attr_t *fill_value; // use for read of not-written data, if set
  smd_attr_t *attr;
  int64_t *actual_size; // used for unlimited dimensions
  esdm_fragments_t fragments;
  int refcount;
  esdm_data_status_e status;
  int mode_flags; // set via esdm_mode_flags_e
};

struct esdm_fragment_t {
  char * id;
  esdm_dataset_t *dataset;
  esdm_dataspace_t *dataspace;
  esdm_backend_t *backend;
  void * backend_md; // backend-specific metadata if set
  void *buf;
  size_t elements;
  size_t bytes;
  //int direct_io;
  esdm_data_status_e status;
};

struct esdm_dataspace_t {
  esdm_type_t type;
  int64_t dims;
  int64_t *size;
  int64_t *offset;
  int64_t *stride;  //may be NULL, in this case contiguous storage in C order is assumed
};

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
  ESDM_MODULE_DATA,
  ESDM_MODULE_METADATA
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
  int (*finalize)(esdm_backend_t * b);
  int (*performance_estimate)(esdm_backend_t * b, esdm_fragment_t *fragment, float *out_time);
  float (*estimate_throughput)(esdm_backend_t * b);
  int (*fragment_create)(esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_retrieve)(esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_update)(esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_delete)(esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_metadata_create)(esdm_backend_t * b, esdm_fragment_t *fragment, smd_string_stream_t* stream);
  void* (*fragment_metadata_load)(esdm_backend_t * b, esdm_fragment_t *fragment, json_t *metadata);
  int (*fragment_metadata_free)(esdm_backend_t * b, void * options);

  int (*mkfs)(esdm_backend_t *, int format_flags);
  int (*fsck)(esdm_backend_t*);
};

struct esdm_md_backend_callbacks_t {
  // General for ESDM
  int (*finalize)(esdm_md_backend_t *);
  int (*performance_estimate)(esdm_md_backend_t *, esdm_fragment_t *fragment, float *out_time);

  // ESDM Data Model Specific
  int (*container_create)(esdm_md_backend_t *, esdm_container_t *container, int allow_overwrite);
  int (*container_commit)(esdm_md_backend_t *, esdm_container_t *container, char * json, int md_size);
  int (*container_retrieve)(esdm_md_backend_t *, esdm_container_t *container, char ** out_json, int * out_size);
  int (*container_update)(esdm_md_backend_t *, esdm_container_t *container);
  int (*container_destroy)(esdm_md_backend_t *, esdm_container_t *container);
  int (*container_remove)(esdm_md_backend_t *, esdm_container_t *container);

  int (*dataset_create)(esdm_md_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_commit)(esdm_md_backend_t *, esdm_dataset_t *dataset, char * json, int md_size);
  int (*dataset_retrieve)(esdm_md_backend_t *, esdm_dataset_t *dataset, char ** out_json, int * out_size);
  int (*dataset_update)(esdm_md_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_destroy)(esdm_md_backend_t *, esdm_dataset_t *dataset);
  int (*dataset_remove)(esdm_md_backend_t *, esdm_dataset_t *dataset);

  int (*mkfs)(esdm_md_backend_t *, int format_flags);
  int (*fsck)(esdm_md_backend_t*);
};

typedef enum esdmI_fragmentation_method_t {
  ESDMI_FRAGMENTATION_METHOD_CONTIGUOUS,  //fragments are optimized for memory locality, may result in fragments that are single slices or even lines of the hypervolume
  ESDMI_FRAGMENTATION_METHOD_EQUALIZED  //all dimensions are treated equally, creating fragments that extend in all available dimensions
} esdmI_fragmentation_method_t;

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
  atomic_int pending_ops;
  GMutex mutex;
  GCond done_condition;
  int return_code;
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
  uint64_t max_fragment_size; //this is a soft limit that may be exceeded anytime
  esdmI_fragmentation_method_t fragmentation_method;
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
} esdm_layout_t;

typedef struct esdm_scheduler_t {
  int info;
  GThreadPool *thread_pool;
  GAsyncQueue *read_queue;
  GAsyncQueue *write_queue;
} esdm_scheduler_t;

typedef struct esdm_performance_t {
  int info;
  } esdm_performance_t;

typedef struct esdm_instance_t esdm_instance_t;

struct esdm_instance_t {
  int is_initialized;
  int procs_per_node;
  int total_procs;
  esdm_statistics_t readStats;
  esdm_statistics_t writeStats;
  esdm_config_t *config;
  esdm_modules_t *modules;
  esdm_layout_t *layout;
  esdm_scheduler_t *scheduler;
  esdm_performance_t *performance;
};

// Auxiliary? /////////////////////////////////////////////////////////////////

typedef struct esdm_bytesequence_t esdm_bytesyquence_t;
struct esdm_bytesequence {
  esdm_type_t type;
  size_t count;
  void *data;
};

typedef struct esdmI_range_t esdmI_range_t;
struct esdmI_range_t {
  int64_t start, end; //start is inclusive, end is exclusive, i.e. the range includes all `x` with `start <= x < end`
};

typedef struct esdmI_hypercube_t esdmI_hypercube_t;
struct esdmI_hypercube_t {
  int64_t dims;
  esdmI_range_t ranges[];
};

//Plain old data object. Neither owns the hypercubes, nor the memory in which their pointers are stored.
typedef struct esdmI_hypercubeList_t {
  esdmI_hypercube_t** cubes;
  int64_t count;
} esdmI_hypercubeList_t;

//A hypercubeList that actually owns its memory, allowing it to grow/shrink as needed.
typedef struct esdmI_hypercubeSet_t esdmI_hypercubeSet_t;
struct esdmI_hypercubeSet_t {
  esdmI_hypercubeList_t list;
  int64_t allocatedCount;
};

#endif
