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



esdm_status_t esdm_init(){
	// for all linked configurations and modules initialize
	// on compile time determined which modules exist
	//return esdm_module_init();

	if (!is_initialized) {
		ESDM_DEBUG(0, "Initializing ESDM.");

		
		esdm_module_init();



		is_initialized = 1;
	}

	return ESDM_SUCCESS;
}

esdm_status_t esdm_finalize(){
	return esdm_module_finalize();
}


esdm_status_t esdm_write(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset)
{
	ESDM_DEBUG(0, "Received write request.");



	esdm_pending_io_t io;
	esdm_scheduler_submit(& io);

	return ESDM_SUCCESS;
}



esdm_status_t esdm_read(void * buf, esdm_dataset_t dset, int dims, uint64_t * size, uint64_t* offset)
{
	ESDM_DEBUG(0, "Received read request.");


	esdm_pending_io_t io;
	esdm_scheduler_submit(& io);


	return ESDM_SUCCESS;
}
