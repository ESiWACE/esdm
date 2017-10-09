#ifndef ESDM_H
#define ESDM_H

#include <esdm-datatypes.h>

/**
* @buff: The pointer to a contiguous memory region that shall be written
* @dset: TODO, currently a stub, we assume it has been identified/created before....
* @dims: The number of dimensions, needed for size and offset
* @size: ...
*/
ESDM_status_t esdm_write(void * buff, ESDM_datatset dset, int dims, uint64_t * size, uint64_t* offset);


#endif
