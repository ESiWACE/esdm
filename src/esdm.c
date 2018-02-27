/* This file is part of ESDM.
 *
 * This program is is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * @file
 * @brief Entry point for ESDM API Implementation
 */


#include <stdio.h>

#include <esdm.h>
#include <esdm-internal.h>




// TODO: Decide on initialization mechanism.
static int is_initialized = 0;
static esdm_instance_t esdm;


/**
* Initialize ESDM:
*	- allocate data structures for ESDM
*	- allocate memory for node local caches
*	- initialize submodules
*	- initialize threadpool
*
* @return status
*/
esdm_status_t esdm_init()
{	
	ESDM_DEBUG(__func__);	

	if (!is_initialized) {
		ESDM_DEBUG("Initializing ESDM.");

		// find configuration
		esdm.config = esdm_config_init(&esdm);

		// optional modules (e.g. data and metadata backends)
		esdm.modules = esdm_modules_init(&esdm);

		// core components
		esdm.layout = esdm_layout_init(&esdm);
		esdm.performance = esdm_performance_init(&esdm);
		esdm.scheduler = esdm_scheduler_init(&esdm);


		printf("[ESDM] esdm = {config = %p, modules = %p, scheduler = %p, layout = %p, performance = %p}\n", esdm.config, esdm.modules, esdm.scheduler, esdm.layout, esdm.performance);
		is_initialized = 1;
	}

	return ESDM_SUCCESS;
}


/**
* Display status information for objects stored in ESDM.
*
* @param [in] desc	name or descriptor of object
*
* @return Status
*/
esdm_status_t esdm_finalize()
{
	ESDM_DEBUG(__func__);	

	// ESDM data data structures that require proper cleanup..
	// in particular this effects data and cache state which is not yet persistent

	//esdm_scheduler_finalize();
	//esdm_performance_finalize();
	//esdm_layout_finalize();
	esdm_modules_finalize();

	return ESDM_SUCCESS;
}






///////////////////////////////////////////////////////////////////////////////
// Application facing API /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




/**
* Display status information for objects stored in ESDM.
*
* @param [in]	desc	name or descriptor of object
* @param [out]	result	where to write result of query
*
* @return Status
*/
esdm_status_t esdm_stat(char *desc, char *result)
{
	ESDM_DEBUG(__func__);	

	esdm_layout_stat(desc);

	return ESDM_SUCCESS;
}


/**
 * Create a new object.
 *
 * @param [in] desc		string object identifier
 * @param [in] mode		mode flags for creation
 *
 * @return Status
 */
esdm_status_t esdm_create(char *desc, int mode)
{
	ESDM_DEBUG(__func__);	


	return ESDM_SUCCESS;
}


/**
 * Open a existing object. (Can be also used to create).
 *
 * @param [in] desc		string object identifier
 * @param [in] mode		mode flags for open/creation
 *
 * @return Status
 */
esdm_status_t esdm_open(char *desc, int mode)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}

/**
 * Write data  of size starting from offset.
 *
 * @param [in] buf	The pointer to a contiguous memory region that shall be written
 * @param [in] dset	TODO, currently a stub, we assume it has been identified/created before...., json description?
 * @param [in] dims	The number of dimensions, needed for size and offset
 * @param [in] size	...
 *
 * @return Status
 */
esdm_status_t esdm_write(void *buf, esdm_dataset_t dset, int dims, uint64_t *size, uint64_t *offset)
{
	ESDM_DEBUG(__func__);	


	esdm_fragment_create();

	esdm_pending_fragment_t fragment;
	esdm_scheduler_submit(& fragment);

	return ESDM_SUCCESS;
}



/**
 * Reads a data fragment described by desc to the dataset dset.
 *
 * @param [out] buf	The pointer to a contiguous memory region that shall be written
 * @param [in] dset	TODO, currently a stub, we assume it has been identified/created before.... , json description?
 * @param [in] dims	The number of dimensions, needed for size and offset
 * @param [in] size	...
 *
 * @return Status
 */
esdm_status_t esdm_read(void *buf, esdm_dataset_t dset, int dims, uint64_t *size, uint64_t *offset)
{
	ESDM_DEBUG(__func__);	

	esdm_pending_fragment_t fragment;
	esdm_scheduler_submit(& fragment);


	return ESDM_SUCCESS;
}



/**
 * Close opened object.
 * 
 * @param [in] desc		String Object Identifier
 *
 * @return Status
 */
esdm_status_t esdm_close(void *desc) 
{
	ESDM_DEBUG(__func__);	


	return ESDM_SUCCESS;
}



/**
 * Ensure all remaining data is syncronized with backends. If not called at the
 * end of an application, ESDM can not guarantee all data was written.
 *
 * @return status
 */
esdm_status_t esdm_sync() 
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}



