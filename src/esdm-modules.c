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
 * @brief ESDM functions for module functionalities.
 *
 */


#include <esdm.h>
#include <esdm-internal.h>


/*
#ifndef ESDM_MODULES_H
#define ESDM_MODULES_H

#ifdef ESDM_HAS_POSIX
#include <data-backends/posix/posix.h>
#endif

static int module_count;
static esdm_module_t modules[] = {
#ifdef ESDM_HAS_POSIX
  & esdm_posix,
#endif
  NULL
};
#endif

static esdm_module_type_array_t modules_per_type[ESDM_TYPE_LAST];
*/


esdm_status_t esdm_module_init() {

	/*
	module_count = 0;
	esdm_module_t * cur = modules;
	for( ; cur != NULL ; cur++){
		cur->init();
		modules_per_type[cur->type()]++;
	}
	*/
	// place the module into the right list
	//
	return ESDM_SUCCESS;
}

esdm_status_t esdm_module_finalize(){
	// reverse finalization of modules
	//
	/*
	for(int i=module_count - 1 ; i >= 0; i--){
		modules[i]->finalize();
	}
	*/
	return ESDM_SUCCESS;
}



esdm_status_t esdm_module_get_by_type(esdm_module_type_t type, esdm_module_type_array_t * array)
{
	return ESDM_SUCCESS;
}




