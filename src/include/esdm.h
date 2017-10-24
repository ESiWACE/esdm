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
*
* @startuml{ZipCmd_ZipComp_Communication.png}
*
* ZipCmd -> ZipComp: First Compute Request
* ZipCmd <-- ZipComp: First Compute Response
*
* ZipCmd -> ZipComp: Second Compute Request
* ZipCmd <-- ZipComp: Second Compute Response
*
* @enduml
*
*
* @param [in] buf	The pointer to a contiguous memory region that shall be written
* @param [in] dset	TODO, currently a stub, we assume it has been identified/created before...., json description?
* @param [in] dims	The number of dimensions, needed for size and offset
* @param [in] size	...
*
* @return Status.
*/
ESDM_status_t esdm_write(void * buf, ESDM_dataset_t dset, int dims, uint64_t * size, uint64_t* offset);



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
ESDM_status_t esdm_read(void * buf, ESDM_dataset_t dset, int dims, uint64_t * size, uint64_t* offset);



ESDM_status_t esdm_lookup(char* desc);













#endif
