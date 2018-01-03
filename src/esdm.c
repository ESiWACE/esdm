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

#include <esdm.h>
#include <esdm-internal.h>




// TODO: Decide on initialization mechanism.
static int is_initialized = 0;



/**
* Display status information for objects stored in ESDM.
*
* @param [in] desc	name or descriptor of object
*
* @return status
*/
esdm_status_t esdm_init()
{	
	ESDM_DEBUG(__func__);	

	// for all linked configurations and modules initialize
	// on compile time determined which modules exist
	//return esdm_module_init();

	if (!is_initialized) {
		ESDM_DEBUG("Initializing ESDM.");

		esdm_scheduler_init();

		esdm_module_init();
		is_initialized = 1;
	}

	return ESDM_SUCCESS;
}


/**
* Display status information for objects stored in ESDM.
*
* @param [in] desc	name or descriptor of object
*
* @return status
*/
esdm_status_t esdm_finalize()
{
	ESDM_DEBUG(__func__);	

	// ESDM data data structures that require proper cleanup..
	// in particular this effects data and cache state which is not yet persistent

	return esdm_module_finalize();
}






///////////////////////////////////////////////////////////////////////////////
// Application facing API /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




/**
* Display status information for objects stored in ESDM.
*
* @param [in] desc	name or descriptor of object
*
* @return TODO
*/
esdm_status_t esdm_stat(char* desc, char* result)
{
	ESDM_DEBUG(__func__);	

	esdm_layout_stat(desc);

	return ESDM_SUCCESS;
}




/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_create(char* desc, int mode)
{
	ESDM_DEBUG(__func__);	


	return ESDM_SUCCESS;
}


/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_open(char* desc, int mode)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}

/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_write(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset)
{
	ESDM_DEBUG(__func__);	


	esdm_fragment_create();

	esdm_pending_fragment_t fragment;
	esdm_scheduler_submit(& fragment);

	return ESDM_SUCCESS;
}



/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_read(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset)
{
	ESDM_DEBUG(__func__);	

	esdm_pending_fragment_t fragment;
	esdm_scheduler_submit(& fragment);


	return ESDM_SUCCESS;
}





/**
* 
*
* @param [in] buf	TODO
*
* @return TODO
*/
esdm_status_t esdm_close(void * buf) 
{
	ESDM_DEBUG(__func__);	


	return ESDM_SUCCESS;
}




