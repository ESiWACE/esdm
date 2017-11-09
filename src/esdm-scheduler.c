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
 * @brief The scheduler receives application requests and schedules subsequent
 *        I/O requests as a are necessary for metadata lookups and data
 *        reconstructions.
 *
 */



#include <esdm.h>
#include <esdm-internal.h>





esdm_status_t esdm_scheduler_init() {


	return ESDM_SUCCESS;
}


esdm_status_t esdm_scheduler_finalize() {
	return ESDM_SUCCESS;
}




esdm_status_t esdm_scheduler_submit(esdm_pending_fragment_t * io) {
	ESDM_DEBUG(0, "Scheduler submit request.");


	esdm_init();


	esdm_pending_fragments_t* pending_fragments;
	esdm_fragment_t* fragments;


	esdm_perf_model_split_io(pending_fragments, fragments);

	// no threads here
	esdm_status_t ret;
	
	//esdm_metadata_t * metadata = esdm_metadata_t_alloc();
	esdm_metadata_t * metadata;


	/*
	for(int i=0 ; i < fragments; i++){
		ret = esdm_backend_io(b_ios[i]->backend, b_ios[i]->io, metadata);
	}
	esdm_metata_backend_update(metadata);
	free(metadata);

	return ESDM_SUCCESS;
	*/
}





esdm_status_t esdm_backend_io(
		esdm_backend_t* backend,
		esdm_fragment_t fragment,
		esdm_metadata_t metadata)
{
	return ESDM_SUCCESS;
}
