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
 * @brief The performance component collects performance estimates from
 *        backends and picks a winner depending on an objective.
 *
 *
 * TODO: cache performance estimates
 *
 *
 *
 *
 *
 */


#include <esdm.h>
#include <esdm-internal.h>

#include <stdio.h>
#include <stdlib.h>


esdm_performance_t* esdm_performance_init(esdm_instance_t* esdm) {
	esdm_performance_t* performance = NULL;
	performance = (esdm_performance_t*) malloc(sizeof(esdm_performance_t));


	// TODO: allocate hash map that serves as perf estimate cache
	//  => backend, estimate_timestamp, estimate question?, an actual estimate


	return performance;
}


esdm_status_t esdm_performance_finalize(esdm_performance_t* performance)
{

	return ESDM_SUCCESS;
}










/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 */
esdm_status_t esdm_performance_recommendation(esdm_fragment_t *in, esdm_fragment_t *out)
{
	ESDM_DEBUG("Fetch performance estimates from backends.");

	// pickup the performance estimate for each backend module
	esdm_module_type_array_t * backends;
	esdm_modules_get_by_type(ESDM_TYPE_DATA, backends);
	//for(int i=0; i < backends->count; i++){
	//	//esdm_backend_estimate_performance((esdm_backend_t*) backends->module, 1234);
	//}

	// now choice the best module
}





esdm_status_t esdm_backend_estimate_performance(esdm_backend_t* backend, int request)
{
	ESDM_DEBUG("Estimate performance call dummy");

	return ESDM_SUCCESS;
}
