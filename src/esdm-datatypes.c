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
 * @brief This file implements ESDM datatypes, and associated methods.
 */


#include <stdlib.h>

#include <esdm.h>
#include <esdm-internal.h>


// Container //////////////////////////////////////////////////////////////////

/**
 * Create a new container. 
 *
 *  - Allocate process local memory structures.
 *	- Register with metadata service.
 *	
 *	@return Pointer to new container.
 *
 */
esdm_container_t* esdm_container_create()
{
	ESDM_DEBUG(__func__);	
	esdm_container_t* new_container = (esdm_container_t*) malloc(sizeof(esdm_container_t));


	return new_container;
}

/**
 * Make container persistent to storage.
 * Schedule for writing to backends.
 */
esdm_status_t esdm_container_commit(esdm_container_t* container)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}


/**
 * Destroy a existing container.
 *
 * Either from memory or from persistent storage.
 *
 */
esdm_status_t esdm_container_destroy(esdm_container_t *container)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}




// Fragment ///////////////////////////////////////////////////////////////////

/**
 * Create a new fragment.
 *
 *  - Allocate process local memory structures.
 *  
 *
 *	A fragment is part of a dataset.
 *
 *	TODO: there should be a mode to auto-commit on creation?
 *
 *	How does this integrate with the scheduler? On auto-commit this merely beeing pushed to sched for dispatch?
 *
 *	
 *	@return Pointer to new fragment.
 *
 */
esdm_fragment_t* esdm_fragment_create()
{
	ESDM_DEBUG(__func__);	
	esdm_fragment_t* new_fragment = (esdm_fragment_t*) malloc(sizeof(esdm_fragment_t));


	return new_fragment;
}


/**
 * Make fragment persistent to storage.
 * Schedule for writing to backends.
 */
esdm_status_t esdm_fragment_commit(esdm_fragment_t *fragment)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}


esdm_status_t esdm_fragment_destroy(esdm_fragment_t *fragment)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}


/**
 * Serializes fragment for storage.
 *
 * @startuml{fragment_serialization.png}
 *
 * User -> Fragment: serialize()
 *
 * Fragment -> Dataspace: serialize()
 * Fragment <- Dataspace: (status, string)
 *
 * User <- Fragment: (status, string)
 *
 * @enduml
 *
 */
esdm_status_t esdm_fragment_serialize(esdm_fragment_t *fragment, char **out)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}


/**
 * Reinstantiate fragment from serialization.
 */
esdm_fragment_t* esdm_fragment_deserialize(char *serialized_fragment)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}




// Dataset ////////////////////////////////////////////////////////////////////
/**
 * Create a new dataset. 
 *
 *  - Allocate process local memory structures.
 *	- Register with metadata service.
 *	
 *	@return Pointer to new dateset.
 *
 */
esdm_dataset_t* esdm_dataset_create()
{
	ESDM_DEBUG(__func__);	
	esdm_dataset_t* new_dataset = (esdm_dataset_t*) malloc(sizeof(esdm_dataset_t));

	return new_dataset;
}

/**
 * Make dataset persistent to storage.
 * Schedule for writing to backends.
 */
esdm_status_t esdm_dataset_commit(esdm_dataset_t *dataset)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}

esdm_status_t esdm_dataset_destroy(esdm_dataset_t *dataset) 
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}


// Dataspace //////////////////////////////////////////////////////////////////

/**
 * Create a new dataspace.
 *
 *  - Allocate process local memory structures.
 *	
 *	@return Pointer to new dateset.
 *
 */
esdm_dataspace_t* esdm_dataspace_create()
{
	ESDM_DEBUG(__func__);	
	esdm_dataspace_t* new_dataspace = (esdm_dataspace_t*) malloc(sizeof(esdm_dataspace_t));


	return new_dataspace;
}

/**
 * Destroy dataspace in memory.
 */
esdm_status_t esdm_dataspace_destroy(esdm_dataspace_t *dataspace)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}

/**
 * Serializes dataspace description.
 *
 * e.g., to store along with fragment
 */

esdm_status_t esdm_dataspace_serialize(esdm_dataspace_t *dataspace, char **out)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}

/**
 * Reinstantiate dataspace from serialization.
 */
esdm_dataspace_t* esdm_dataspace_deserialize(char *serialized_dataspace)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}
