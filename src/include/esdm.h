/**
 * @file
 * @brief Public API of the ESDM. Inlcudes several other public interfaces.
 */
#ifndef ESDM_H
#define ESDM_H

#include <stddef.h>

#include <esdm-datatypes.h>




///////////////////////////////////////////////////////////////////////////////
// ESDM ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/**
* Initialize ESDM:
*	- allocate data structures for ESDM
*	- allocate memory for node local caches
*	- initialize submodules
*	- initialize threadpool
*
* @return status
*/
esdm_status_t esdm_init();


/**
* Initialize ESDM:
*  - finalize submodules
*  - free data structures
*
* @return status
*/
esdm_status_t esdm_finalize();




///////////////////////////////////////////////////////////////////////////////
// Application facing API /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_create(char* desc, int mode);



/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_open(char* desc, int mode);


/**
* Display status information for objects stored in ESDM.
*
* @param [in] desc	name or descriptor of object
*
* @return TODO
*/
esdm_status_t esdm_stat(char* desc, char* result);


/**
*
* @param [in] buf	The pointer to a contiguous memory region that shall be written
* @param [in] dset	TODO, currently a stub, we assume it has been identified/created before...., json description?
* @param [in] dims	The number of dimensions, needed for size and offset
* @param [in] size	...
*
* @return Status.
*/
esdm_status_t esdm_write(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset);


/**
* Reads a data fragment described by desc to the dataset dset.
*
* @param [out] buf	The pointer to a contiguous memory region that shall be written
* @param [in] dset	TODO, currently a stub, we assume it has been identified/created before.... , json description?
* @param [in] dims	The number of dimensions, needed for size and offset
* @param [in] size	...
*
* @return Status.
*/
esdm_status_t esdm_read(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset);


/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_close(void * buf);












#endif
