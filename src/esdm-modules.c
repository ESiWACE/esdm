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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esdm.h>
#include <esdm-internal.h>


#define ESDM_HAS_POSIX
//#define ESDM_HAS_CLOVIS
//#define ESDM_HAS_WOS


#define ESDM_HAS_METADUMMY



#ifdef ESDM_HAS_POSIX
	#include "backends-data/POSIX/posix.h"
	#pragma message ("Building ESDM with POSIX!")
#endif

#ifdef ESDM_HAS_CLOVIS
	#include "backends-data/Clovis/clovis.h"
	#pragma message ("Building ESDM with Clovis!")
#endif

#ifdef ESDM_HAS_WOS
	#include "backends-data/Mero/wos.h"
	#pragma message ("Building ESDM with WOS!")
#endif



#ifdef ESDM_HAS_METADUMMY
	#include "backends-metadata/metadummy/metadummy.h"
	#pragma message ("Building ESDM with metadummy!")
#endif




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






esdm_modules_t* esdm_modules_init(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);	

	esdm_modules_t* modules = NULL;
	modules = (esdm_modules_t*) malloc(sizeof(esdm_modules_t));

	modules->bcount = 0;
	modules->mcount = 0;

	esdm_config_backends_t* config_backends = esdm_config_get_backends(esdm);
	esdm_config_backend_t* b = NULL;


	modules->bcount = config_backends->count;
	modules->backends = (esdm_backend_t**) malloc(sizeof(esdm_backend_t*)*(modules->bcount));

	
	// add a metadummy backend
	modules->metadata = metadummy_backend_init(NULL);


	for (int i = 0; i < modules->bcount; i++) {
		b = &(config_backends->backends[i]);

		printf("Backend config: %d, %s, %s, %s\n", i,
				b->type,	
				b->name,	
				b->target
			  );

		if (strncmp (b->type,"POSIX",5) == 0)
		{
			posix_backend_options_t* data = (posix_backend_options_t*) malloc(sizeof(posix_backend_options_t));
			data->type = b->type;
			data->name = b->name;
			data->target = b->target;

			modules->backends[i] = posix_backend_init((void*)data);
			modules->backends[i]->callbacks.performance_estimate(modules->backends[i]);
		} else {
			ESDM_ERROR("Unknown backend type. Please check your ESDM configuration.");
		}

	}






	// place the module into the right list
	//
	return modules;
}

esdm_status_t esdm_modules_finalize()
{
	ESDM_DEBUG(__func__);	

	// reverse finalization of modules
	//
	/*
	for(int i=module_count - 1 ; i >= 0; i--){
		modules[i]->finalize();
	}
	*/
	return ESDM_SUCCESS;
}



esdm_status_t esdm_modules_get_by_type(esdm_module_type_t type, esdm_module_type_array_t * array)
{
	ESDM_DEBUG(__func__);	







	return ESDM_SUCCESS;
}




