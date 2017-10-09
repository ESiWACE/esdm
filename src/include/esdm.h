/**
 * @file
 * @brief Public API of the ESDM. Inlcudes several other public interfaces.
 */
#ifndef ESDM_H
#define ESDM_H

#include <esdm-datatypes.h>
#include <esdm-data-backend.h>
#include <esdm-metadata-backend.h>

/**
* @param buff The pointer to a contiguous memory region that shall be written
* @param dset TODO, currently a stub, we assume it has been identified/created before....
* @param dims The number of dimensions, needed for size and offset
* @param size ...
*
* @return Status.
*/
ESDM_status_t esdm_write(void * buff, ESDM_datatset dset, int dims, uint64_t * size, uint64_t* offset);


#endif
