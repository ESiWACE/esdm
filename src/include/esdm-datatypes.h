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
  ESDM_INVALID_ARGUMENT_ERROR,
  ESDM_INVALID_PERMISSIONS,
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
typedef struct esdm_md_backend_callbacks_t esdm_md_backend_callbacks_t;
typedef struct esdm_md_backend_t esdm_md_backend_t;

#endif
