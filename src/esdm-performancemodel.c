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
 */


#include <esdm.h>
#include <esdm-internal.h>


ESDM_status_t esdm_performance_init() {
	return ESDM_SUCCESS;
}


ESDM_status_t esdm_performance_finalize() {
	return ESDM_SUCCESS;
}

esdm_perf_model_split_io(io){
	// pickup the performance estimate for each backend module
	esdm_module_type_array_t * backends = esdm_module_get_per_type(ESDM_TYPE_BACKEND);
	for(int i=0; i < backends->count; i++){
		esdm_backend_estimate_performance((esdm_backend_t*) backends->module, io);
	}
	// now choice the best module
}
