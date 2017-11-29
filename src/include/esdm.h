/**
 * @file
 * @brief Public API of the ESDM. Inlcudes several other public interfaces.
 */
#ifndef ESDM_H
#define ESDM_H

#include <stddef.h>



#include <esdm-datatypes.h>
#include <esdm-data-backend.h>
#include <esdm-metadata-backend.h>


typedef int esdm_type;

// ESDM
esdm_status_t esdm_init();


// Module


esdm_status_t esdm_module_init();
esdm_status_t esdm_module_finalize();
esdm_status_t esdm_module_register();



esdm_status_t esdm_module_get_by_type(esdm_module_type_t type, esdm_module_type_array_t * array);













// API to be used by applications
esdm_status_t esdm_lookup(char* desc);


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











#endif
