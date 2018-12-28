/**
 * @file
 * @brief Datatype primitives provided by ESDM.
 */
#ifndef ESDM_DATATYPES_H
#define ESDM_DATATYPES_H


#include <stdint.h>

#define ESDM_MAX_SIZE 1024

typedef int esdm_type;

// ESDM Parameters and Status /////////////////////////////////////////////////
//
/**
 * ESDM Status codes and failure modes.
 */
typedef enum esdm_mode  {
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

typedef enum esdm_datatype {
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
} esdm_datatype;





/**
 * ESDM Status codes and failure modes.
 */
typedef enum esdm_status {
	ESDM_SUCCESS = 0,
	ESDM_ERROR,
	ESDM_STATUS_DIRTY,
	ESDM_STATUS_PERSISTENT
} esdm_status;

/**
 * ESDM provides logging helpers, the available loglevels are defined here.
 */
typedef enum esdm_loglevel {
	ESDM_LOGLEVEL_CRITICAL,
	ESDM_LOGLEVEL_ERROR,
	ESDM_LOGLEVEL_WARNING,
	ESDM_LOGLEVEL_INFO,
	ESDM_LOGLEVEL_DEBUG,
	ESDM_LOGLEVEL_NOTSET
} esdm_loglevel;


// LOGICAL/DOMAIN DATATYPES ///////////////////////////////////////////////////

typedef struct esdm_container esdm_container;
typedef struct esdm_metadata esdm_metadata;
typedef struct esdm_dataset_t esdm_dataset_t;
typedef struct esdm_dataspace_t esdm_dataspace_t;
typedef struct esdm_fragment_t esdm_fragment_t;
typedef struct esdm_backend esdm_backend;
typedef struct esdm_backend_callbacks_t esdm_backend_callbacks_t;



#endif
