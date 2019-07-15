/**
 * @file
 * @brief Datatype primitives provided by ESDM.
 */
#ifndef ESDM_DATATYPES_H
#define ESDM_DATATYPES_H

#include <smd.h>

#define ESDM_MAX_SIZE 1024

typedef smd_dtype_t *esdm_type_t;

// ESDM Parameters and Status /////////////////////////////////////////////////
//
/**
 * ESDM Status codes and failure modes.
 */
typedef enum esdm_mode {
  ESDM_OVERWRITE,
  ESDM_CREATE,
  ESDM_AUTOCOMMIT,
  ESDM_DATASET,
  ESDM_CONTAINER,
} esdm_mode;

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
  ESDM_INVALID_ARGUMENT_ERROR,
  ESDM_ERROR
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

typedef struct esdm_container_t esdm_container_t;
typedef struct esdm_datasets_t esdm_datasets_t;
typedef struct esdm_dataset_t esdm_dataset_t;
typedef struct esdm_dataspace_t esdm_dataspace_t;
typedef struct esdm_fragment_t esdm_fragment_t;
typedef struct esdm_backend_t esdm_backend_t;
typedef struct esdm_md_backend_t esdm_md_backend_t;
typedef struct esdm_config_backend_t esdm_config_backend_t;
typedef struct esdm_backend_t_callbacks_t esdm_backend_t_callbacks_t;
typedef struct esdm_md_backend_callbacks_t esdm_md_backend_callbacks_t;
typedef struct esdm_attr_t esdm_attr_t;
typedef struct esdm_attr_group_t esdm_attr_group_t;
typedef struct esdm_dataset_iterator_t esdm_dataset_iterator_t;

struct esdm_dataspace_t {
  esdm_type_t type;
  int64_t dims;
  int64_t *size;
  esdm_dataspace_t *subspace_of;
  int64_t *offset;
  int64_t *stride;  //may be NULL, in this case contiguous storage in C order is assumed
};

#endif
