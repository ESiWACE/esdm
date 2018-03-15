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
esdm_instance_t esdm;


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
		ESDM_DEBUG("Initializing ESDM");

		// find configuration
		esdm.config = esdm_config_init(&esdm);

		// optional modules (e.g. data and metadata backends)
		esdm.modules = esdm_modules_init(&esdm);

		// core components
		esdm.layout = esdm_layout_init(&esdm);
		esdm.performance = esdm_performance_init(&esdm);
		esdm.scheduler = esdm_scheduler_init(&esdm);


		printf("[ESDM] esdm = {config = %p, modules = %p, scheduler = %p, layout = %p, performance = %p}\n", 
						  (void*)esdm.config,
						  (void*)esdm.modules,
						  (void*)esdm.scheduler,
						  (void*)esdm.layout, 
						  (void*)esdm.performance);

		is_initialized = 1;

		ESDM_DEBUG("ESDM initialized and ready!");
		printf("\n");
	}

	return ESDM_SUCCESS;
}


/**
* Display status information for objects stored in ESDM.
*
* @param [in] desc	Name or descriptor of object.
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
// Public API: POSIX Legacy Compaitbility /////////////////////////////////////
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

	esdm_init();

	esdm_layout_stat(desc);

	return ESDM_SUCCESS;
}


/**
 * Create a new object.
 *
 * @param [in]	desc		string object identifier
 * @param [in]	mode		mode flags for creation
 * @param [out] container	pointer to new container
 *
 * @return Status
 */
esdm_status_t esdm_create(char *name, int mode, esdm_container_t **container)
{
	ESDM_DEBUG(__func__);	

	esdm_init();

	*container = esdm_container_create(name);
	esdm_dataspace_t* dataspace = esdm_dataspace_create(1 /* dims */);
	esdm_dataset_t* dataset = esdm_dataset_create(*container, "bytestream", dataspace);
	
	printf("Dataset 'bytestream' creation: %p\n", dataset);

	esdm_dataset_commit(dataset);
	esdm_container_commit(*container);

	return ESDM_SUCCESS;
}


/**
 * Open a existing object.
 *
 * TODO: decide if also useable to create?
 *
 * @param [in] desc		string object identifier
 * @param [in] mode		mode flags for open/creation
 *
 * @return Status
 */
esdm_status_t esdm_open(char *name, int mode)
{
	ESDM_DEBUG(__func__);	

	esdm_init();

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

esdm_status_t esdm_write(esdm_container_t *container, void *buf, int dims, uint64_t * size, uint64_t* offset)
{
	ESDM_DEBUG(__func__);	

	esdm_dataset_t *dataset = (esdm_dataset_t*) g_hash_table_lookup (container->datasets, "bytestream");
	printf("Dataset 'bytestream' lookup: %p\n", dataset);

	// create new fragment
	esdm_dataspace_t *subspace = esdm_dataspace_create();
	esdm_fragment_t *fragment = esdm_fragment_create(dataset, subspace, buf);

	// schedule for I/O
	esdm_scheduler_enqueue(&esdm, fragment);

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
esdm_status_t esdm_read(esdm_container_t *container, void *buf, int dims, uint64_t * size, uint64_t* offset)
{
	ESDM_DEBUG(__func__);	

	esdm_dataset_t *dataset = (esdm_dataset_t*) g_hash_table_lookup (container->datasets, "bytestream");
	printf("Dataset 'bytestream' lookup: %p\n", dataset);

	// TODO: reconstruct expected result using esdm_layout

	//esdm_scheduler_enqueue(fragment);
	
	// buf =

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
 * Ensure all remaining data is syncronized with backends.
 * If not called at the end of an application, ESDM can not guarantee all data
 * was written.
 *
 * @return status
 */
esdm_status_t esdm_sync() 
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}



