#ifndef h5_sqlite_plugin_h
#define h5_sqlite_plugin_h


#include <unistd.h>
#include <sys/types.h>
#include <hdf5.h>


#define FILE_DEFAULT_PATH "/root"



typedef struct h5sqlite_fapl_t {
  int mpi_size;
  int mpi_rank;
	char* fn;
  char* db_fn;
  char* data_fn;
} h5sqlite_fapl_t;

struct SQF_t;

typedef struct SQO_t {
	char* location;
	char* name;
	H5O_info_t info;
	struct SQF_t* root;
	h5sqlite_fapl_t* fapl;
} SQO_t;    /* structure for object*/

typedef struct SQF_t {
	SQO_t object;
	int fd;
  off64_t offset; // global offset
#ifdef SMARTFILE
	int fd_shm;
  off64_t offset_shm;
	int fd_ssh;
  off64_t offset_ssd;
#endif
	void* db;
} SQF_t;    /* structure for file*/

typedef struct SQD_t {
	SQO_t object;
	off64_t offset; // position in file
	size_t data_size;
} SQD_t;     /* structure for dataset*/

typedef struct SQG_t {
	SQO_t object;
} SQG_t;     /* structure for group*/

typedef struct SQA_t {
	SQO_t object;
	size_t data_size;
	size_t corder; // creation order
} SQA_t;     /*structure for attribute*/


typedef enum H5VL_object_optional_t {
	H5VL_OBJECT_GET_COMMENT,         /* get object comment                */
	H5VL_OBJECT_GET_INFO,        /* get object info                   */
	H5VL_OBJECT_SET_COMMENT            /* set object comment                */
} H5VL_object_optional_t;


#endif
