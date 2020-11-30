/**
 * @file
 * @brief Datatype primitives provided by ESDM.
 */
#ifndef ESDM_DATATYPES_H
#define ESDM_DATATYPES_H

#include <smd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESDM_MAX_SIZE 1024

typedef smd_dtype_t *esdm_type_t;

// ESDM Parameters and Status /////////////////////////////////////////////////
//
/**
 * ESDM Status codes and failure modes.
 */
typedef enum {
  ESDM_MODE_FLAG_WRITE = 1,
  ESDM_MODE_FLAG_READ = 2
} esdm_mode_flags_e;

// where is the data accessible
typedef enum data_accessibility_t {
  ESDM_ACCESSIBILITY_GLOBAL, // shared file system etc.
  ESDM_ACCESSIBILITY_NODELOCAL
} data_accessibility_t;

/**
 * ESDM Status codes and failure modes.
 */
typedef enum esdm_status {
  ESDM_SUCCESS = 0,
  ESDM_ERROR, //some unspecific error, used when none of the below fits

  ESDM_INVALID_ARGUMENT_ERROR,
  ESDM_INVALID_STATE_ERROR, //returned when the state of an object does not allow the attempted operation
  ESDM_INVALID_DATA_ERROR,  //returned when some input data is in an inconsistent state
  ESDM_INVALID_PERMISSIONS,
  ESDM_INCOMPLETE_DATA, //returned when a read requests data that does not exist and no fill-value is set for the dataset
  ESDM_DIRTY_DATA_ERROR,  //attempt to read data from disk that's been modified in memory, the read would discard the in-memory changes
  ESDM_DELETED_DATA_ERROR   //attempt to access data that has been deleted from disk
} esdm_status;

/**
 * ESDM provides logging helpers, the available loglevels are defined here.
 */
enum esdm_loglevel {
  ESDM_LOGLEVEL_NOTSET,
  ESDM_LOGLEVEL_ERROR,
  ESDM_LOGLEVEL_WARNING,
  ESDM_LOGLEVEL_INFO,
  ESDM_LOGLEVEL_DEBUG
};
typedef enum esdm_loglevel esdm_loglevel_e;

// LOGICAL/DOMAIN DATATYPES ///////////////////////////////////////////////////

typedef struct esdm_attr_group_t esdm_attr_group_t;
typedef struct esdm_attr_t esdm_attr_t;
typedef struct esdm_backend_t esdm_backend_t;
typedef struct esdm_backend_t_callbacks_t esdm_backend_t_callbacks_t;
typedef struct esdm_config_backend_t esdm_config_backend_t;
typedef struct esdm_container_t esdm_container_t;
typedef struct esdm_dataset_iterator_t esdm_dataset_iterator_t;
typedef struct esdm_dataset_t esdm_dataset_t;
typedef struct esdm_datasets_t esdm_datasets_t;
typedef struct esdm_dataspace_t esdm_dataspace_t;
typedef struct esdm_fragment_t esdm_fragment_t;
typedef struct esdm_grid_t esdm_grid_t;
typedef struct esdm_gridIterator_t esdm_gridIterator_t;
typedef struct esdm_md_backend_callbacks_t esdm_md_backend_callbacks_t;
typedef struct esdm_md_backend_t esdm_md_backend_t;

//This needs to be public to allow creating simple dataspaces.
struct esdm_dataspace_t {
  esdm_type_t type;
  int64_t dims;
  int64_t *size;
  int64_t *offset;
  int64_t *strideBacking; //May hold a sufficiently large memory buffer to be used in case that a stride is set. This is used for simple dataspaces which must use a stack based stride.
  int64_t *stride;  //may be NULL, in this case contiguous storage in C order is assumed
};

//Facilitate creating small dataspace objects on the stack.
//To be used with one of the esdm_dataspace_<N>d[o]() constructor macros.
typedef struct esdm_simple_dspace_t {
  esdm_dataspace_t* ptr;
} esdm_simple_dspace_t;

typedef struct scil_user_hints_t scil_user_hints_t;

/**
 * This POD struct is used to return a bunch of statistics to the user.
 */
typedef struct esdm_statistics_t {
  uint64_t bytesUser; //the amount of bytes that the user requested to be read/written
  uint64_t bytesInternal; //the amount of bytes that were read/written due to an internal request
  uint64_t bytesIo; //the amount of bytes that actually were read/written from/to storage hardware
  uint64_t requests;  //the amount of read/write requests issued by the user
  uint64_t internalRequests;  //the amount of internal read/write requests that were generated
  uint64_t fragments; //the amount of data object actually read/written from/to storage hardware
} esdm_statistics_t;

#ifdef __cplusplus
}
#endif


#endif
