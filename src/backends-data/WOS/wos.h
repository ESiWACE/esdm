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
 *
 */
#ifndef ESDM_BACKENDS_WOS_H
#define ESDM_BACKENDS_WOS_H

#include <esdm.h>
#include <backends-data/generic-perf-model/lat-thr.h>

#include "wrapper/wos_wrapper.h"

// Internal functions used by this backend.
typedef struct {

	esdm_config_backend_t *config;
	const char *target;
	esdm_perf_model_lat_thp_t perf_model;

	t_WosClusterPtr *wos_cluster;	// Pointer to WOS cluster
	t_WosPolicy *wos_policy;	// Policy
	t_WosObjPtr *wos_meta_obj;	// Used for internal purposes to store metadata info
	t_WosOID **oid_list;	// List of object ids
	uint64_t *size_list;	// List of object sizes

} wos_backend_data_t;
typedef wos_backend_data_t esdm_backend_wos_t;

esdm_backend_t *wos_backend_init(esdm_config_backend_t * config);

extern esdm_backend_wos_t esdm_backend_wos;

#endif
