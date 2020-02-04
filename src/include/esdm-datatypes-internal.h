#ifndef ESDM_DATATYPES_INTERNAL_H
#define ESDM_DATATYPES_INTERNAL_H

#include <jansson.h>
#include <glib.h>
#include <stdatomic.h>
#include <stdbool.h>

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

struct esdm_fragments_vtable_t {
  void (*add)(void* me, esdm_fragment_t* fragment);  //takes possession of the fragment, eventually calling `esdm_fragment_destroy()` on it when the `void` object is destructed
  esdm_fragment_t** (*list)(void* me, int64_t* out_fragmentCount); //returns a pointer to internal storage
  esdm_fragment_t** (*makeSetCoveringRegion)(void* me, esdmI_hypercube_t* region, int64_t* out_fragmentCount);  //caller is responsible to free the returned array
  void (*metadata_create)(void* me, smd_string_stream_t* s);
  esdm_status (*destruct)(void* me);  //calls `esdm_fragment_destroy()` on its members, but does not invoke the `fragment_delete()` callback of the backend
};

//abstract base class to handle the list of fragments associated with a dataset
struct esdm_fragments_t {
  struct esdm_fragments_vtable_t* vtable;
};

typedef struct esdmI_hypercubeNeighbourManager_t esdmI_hypercubeNeighbourManager_t;
//uses an esdmI_hypercubeNeighbourManaget_t to quickly decide which fragments are used to cover a given region
struct esdmI_neighbourFragments_t {
  esdm_fragments_t super;
  esdm_fragment_t ** frag;
  esdmI_hypercubeNeighbourManager_t* neighbourManager;
  int count;
  int buff_size;
};


struct esdm_dataset_t {
  char *name;
  char *id;
  char **dims_dset_id; // array of variable names != NULL if set
  esdm_container_t *container;
  esdm_dataspace_t *dataspace;
  smd_attr_t *fill_value; // use for read of not-written data, if set
  smd_attr_t *attr;
  int64_t *actual_size; // used for unlimited dimensions
  esdm_fragments_t* fragments;
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

  int   (*performance_estimate)(esdm_backend_t * b, esdm_fragment_t *fragment, float *out_time);
  float (*estimate_throughput) (esdm_backend_t * b);

  int (*fragment_create)  (esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_retrieve)(esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_update)  (esdm_backend_t * b, esdm_fragment_t *fragment);
  int (*fragment_delete)  (esdm_backend_t * b, esdm_fragment_t *fragment);

  int (*fragment_metadata_create)(esdm_backend_t * b, esdm_fragment_t *fragment, smd_string_stream_t* stream);
  void* (*fragment_metadata_load)(esdm_backend_t * b, esdm_fragment_t *fragment, json_t *metadata);
  int (*fragment_metadata_free) (esdm_backend_t * b, void * options);

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
  void *data;    /* backend-specific data. */
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
  esdm_config_backend_t **backends;
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

enum {
  BOUND_LIST_IMPLEMENTATION_ARRAY = 0,
  BOUND_LIST_IMPLEMENTATION_BTREE = 1
};

typedef struct esdm_config_t {
  void *json;
  uint8_t boundListImplementation;  //one of the BOUND_LIST_IMPLEMENTATION_* constants
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

//helper for the esdmI_boundList_t implementations
typedef struct esdmI_boundListEntry_t esdmI_boundListEntry_t;
struct esdmI_boundListEntry_t {
  int64_t bakedBound;  //the LSB is used to store whether the bound is a start or end bound (1 == start bound), the actual bound is shifted left one bit
  int64_t cubeIndex;
};

typedef struct esdmI_boundTree_t esdmI_boundTree_t;
typedef union esdmI_boundIterator_t {
  struct {
    esdmI_boundListEntry_t* entry;
  } arrayIterator;
  struct {
    esdmI_boundTree_t* node;
    int entryPosition;
  } treeIterator;
} esdmI_boundIterator_t;

typedef struct esdmI_boundList_vtable_t esdmI_boundList_vtable_t;
struct esdmI_boundList_vtable_t {
  void (*add)(void* me, int64_t bound, bool isStart, int64_t cubeIndex);
  esdmI_boundListEntry_t* (*findFirst)(void* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator);
  esdmI_boundListEntry_t* (*nextEntry)(void* me, esdmI_boundIterator_t* inout_iterator);
  void (*destruct)(void* me);
};

typedef struct esdmI_boundList_t esdmI_boundList_t;
struct esdmI_boundList_t {
  esdmI_boundList_vtable_t* vtable;
};

static inline void boundList_add(esdmI_boundList_t* me, int64_t bound, bool isStart, int64_t cubeIndex) { me->vtable->add(me, bound, isStart, cubeIndex); }
static inline esdmI_boundListEntry_t* boundList_findFirst(esdmI_boundList_t* me, int64_t bound, bool isStart, esdmI_boundIterator_t* out_iterator) { return me->vtable->findFirst(me, bound, isStart, out_iterator); }
static inline esdmI_boundListEntry_t* boundList_nextEntry(esdmI_boundList_t* me, esdmI_boundIterator_t* inout_iterator) { return me->vtable->nextEntry(me, inout_iterator); }
static inline void boundList_destruct(esdmI_boundList_t* me) { me->vtable->destruct(me); }

//helper for esdmI_hypercubeNeighbourManager_t
//The intention of implementing this as a class of its own is to facilitate changing the data structure
//from a linear sorted list with binary search to some balanced tree in the future.
//
//This is private to esdmI_hypercubeNeighbourManager_t.
typedef struct esdmI_boundArray_t esdmI_boundArray_t;
struct esdmI_boundArray_t {
  esdmI_boundList_t super;
  esdmI_boundListEntry_t* entries;
  int64_t count, allocatedCount;
};

//Stores an index of bounds in the form of a B-tree.
//The max of 21 puts the sizeof(esdmI_boundTree_t) at 512 bytes, which is exactly eight cache lines.
//This is a tuning parameter that might call for other values on other machines than mine.
//
//XXX: The motivation for this rather complex structure over the simple array in `esdmI_boundArray_t` is that the later has a quadratic complexity.
//     While the simple array access outperforms the more complicated data structure for small hypercube counts,
//     the B-tree outperforms the simply array when we have a couple of thousands entries in the list.
//     We simply cannot tolerate quadratic complexities when the N is controlled by HPC applications...
#define BOUND_TREE_MAX_BRANCH_FACTOR 21
#define BOUND_TREE_MAX_ENTRY_COUNT (BOUND_TREE_MAX_BRANCH_FACTOR - 1)
struct esdmI_boundTree_t {
  esdmI_boundList_t super;
  int64_t entryCount;
  esdmI_boundListEntry_t bounds[BOUND_TREE_MAX_ENTRY_COUNT];
  esdmI_boundTree_t* children[BOUND_TREE_MAX_BRANCH_FACTOR];
  esdmI_boundTree_t* parent;
};

//another helper for esdmI_hypercubeNeighbourManager_t
typedef struct esdmI_neighbourList_t esdmI_neighbourList_t;
struct esdmI_neighbourList_t {
  int64_t neighbourCount, allocatedCount, *neighbourIndices;
};

//A hypercubeList that owns its memory, and which keeps track of the neighbourhood relations between the different hypercubes.
struct esdmI_hypercubeNeighbourManager_t {
  esdmI_hypercubeList_t list;
  int64_t allocatedCount;
  int64_t dims; //All hypercubes in the list must be of the same rank.

  esdmI_neighbourList_t* neighbourLists;  //`list->count` entries, space for `allocatedCount` entries

  esdmI_boundList_t* boundLists[];  //one esdmI_boundList_t per dimension
};

typedef struct esdm_readTimes_t esdm_readTimes_t;
struct esdm_readTimes_t {
  double makeSet; //the time to determine the sets of fragments than need to be fetched from disk
  double coverageCheck; //the time needed to check whether the available fragments cover the requested regions
  double enqueue; //the time needed to queue the read requests
  double completion;  //the time spent waiting for background tasks to complete
  double writeback; //the time spent writing back fragments after transposition/composition into the user requested data layout
  double total; //sum of all the times above and other small things like taking times...
};

typedef struct esdm_writeTimes_t esdm_writeTimes_t;
struct esdm_writeTimes_t {
  double backendDistribution; //the time spent deciding which backend should handle which parts of the data
  double backendDispatch; //the time spent sending fragments to the different backends
  double completion;  //the time spent waiting for background tasks to complete
  double total; //sum of all the times above and other small things like taking times...
};

#endif
