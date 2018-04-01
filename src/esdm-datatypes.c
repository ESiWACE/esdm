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

#define _GNU_SOURCE         /* See feature_test_macros(7) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <esdm.h>
#include <esdm-internal.h>



extern esdm_instance_t esdm;





// Native Datatypes ///////////////////////////////////////////////////////////
size_t esdm_sizeof(esdm_datatype_t type) {
	switch (type) {
		case esdm_int8_t:
			return sizeof(int8_t);
		case esdm_int16_t:
			return sizeof(int16_t);
		case esdm_int32_t:
			return sizeof(int32_t);
		case esdm_int64_t:
			return sizeof(int64_t);

		case esdm_uint8_t:
			return sizeof(uint8_t);
		case esdm_uint16_t:
			return sizeof(uint16_t);
		case esdm_uint32_t:
			return sizeof(uint32_t);
		case esdm_uint64_t:
			return sizeof(uint64_t);

		case esdm_float:		// if IEEE 754 (32bit)
			return sizeof(float);
		case esdm_double:		// if IEEE 754 (64bit)
			return sizeof(double);

		// TODO: is a native char type really needed?
		case esdm_char:
		case esdm_char_ascii:
			return 1;

	}
}









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
esdm_container_t* esdm_container_create(const char* name)
{
	ESDM_DEBUG(__func__);	
	esdm_container_t* container = (esdm_container_t*) malloc(sizeof(esdm_container_t));

	asprintf(&container->name, name);
	container->metadata = NULL;
	container->datasets = g_hash_table_new(g_direct_hash,  g_direct_equal);
	container->status = ESDM_DIRTY;

	return container;
}


esdm_container_t* esdm_container_retrieve(const char * name)
{
	ESDM_DEBUG(__func__);	
	esdm_container_t* container = (esdm_container_t*) malloc(sizeof(esdm_container_t));

	// TODO: retrieve from MD
	// TODO: retrieve associated data

	asprintf(&container->name, name);
	container->metadata = NULL;
	container->datasets = g_hash_table_new(g_direct_hash,  g_direct_equal);
	container->status = ESDM_DIRTY;

	return container;
}



/**
 * Make container persistent to storage.
 * Schedule for writing to backends.
 *
 * Calling container commit may trigger subsequent commits for datasets that
 * are part of the container.
 *
 */
esdm_status_t esdm_container_commit(esdm_container_t* container)
{
	ESDM_DEBUG(__func__);	

	// print datasets of this container
	g_hash_table_foreach (container->datasets, print_hashtable_entry, NULL);


	// TODO: ensure callback is not NULL
	// md callback create/update container
	esdm.modules->metadata->callbacks.container_create(esdm.modules->metadata, container);
	
	
	// Also commit uncommited datasets of this container?
	//g_hash_table_foreach (container->datasets, /* TODO: dataset commit wrapper? */ print_hashtable_entry, NULL);


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
esdm_fragment_t* esdm_fragment_create(esdm_dataset_t* dataset, esdm_dataspace_t* subspace, void *data)
{
	ESDM_DEBUG(__func__);	
	esdm_fragment_t* fragment = (esdm_fragment_t*) malloc(sizeof(esdm_fragment_t));

	
	for (int64_t i = 0; i < subspace->dimensions; i++) { printf("dim %d, subsize=%d (%p)\n", i, subspace->subsize[i], subspace->subsize); }

	// calculate subspace element count
	int64_t size = 0;
	for (int64_t i = 0; i < subspace->dimensions; i++)
	{
		if (size == 0 && subspace->subsize[i] > 0)
		{
			size += subspace->subsize[i];
		}
		else if(subspace->subsize[i] > 0)
		{
			size *= subspace->subsize[i];
		}
	}

	int64_t bytes = size*esdm_sizeof(subspace->datatype);
	printf("Entries in subspace: %d x %d bytes = %d bytes \n", size, esdm_sizeof(subspace->datatype), bytes);


	fragment->metadata = NULL;
	fragment->dataset = dataset;
	fragment->dataspace = subspace;
	fragment->data = data;	// zero copy?
	fragment->size = size;
	fragment->bytes = bytes;
	fragment->status = ESDM_DIRTY;


	esdm_dataspace_string_descriptor(fragment->dataspace);

	return fragment;
}



esdm_fragment_t* esdm_fragment_retrieve(esdm_container_t *container, esdm_dataset_t *dataset, const char *id)
{
	ESDM_DEBUG(__func__);	
	esdm_fragment_t* fragment = (esdm_fragment_t*) malloc(sizeof(esdm_fragment_t));

	fragment->dataset = dataset;


	return fragment;
}


char* esdm_dataspace_string_descriptor(esdm_dataspace_t *dataspace)
{
	ESDM_DEBUG(__func__);	

	char *string = NULL;
	char *string_size = NULL;
	char *string_offset = NULL;

	int64_t dimensions = dataspace->dimensions;
	int64_t *size = dataspace->subsize;
	int64_t *offset = dataspace->offset;

	// offset to string
	for (int64_t i = 0; i < dimensions; i++)
	{
		printf("dim %d, offset=%d (%p)\n", i, offset[i], offset);

		if (string_offset == NULL)
			asprintf(&string_offset, "%d", offset[i]);
		else
			asprintf(&string_offset, "%s,%d", string_offset, offset[i]);
	}

	// size to string
	for (int64_t i = 0; i < dimensions; i++)
	{
		printf("dim %d, size=%d (%p)\n", i, size[i], size);

		if (string_size == NULL)
			asprintf(&string_size, "%d", size[i]);
		else
			asprintf(&string_size, "%s,%d", string_size, size[i]);
	}

	// combine offset + size
	asprintf(&string, "%s_%s", string_offset, string_size);
	printf("Descriptor: %s\n", string);

	free(string_size);
	free(string_offset);

	return string;
}




/**
 * Make fragment persistent to storage.
 * Schedule for writing to backends.
 */
esdm_status_t esdm_fragment_commit(esdm_fragment_t *fragment)
{
	ESDM_DEBUG(__func__);	

	esdm_status_t status;

	// schedule for I/O
	esdm_scheduler_enqueue(&esdm, fragment);

	// Call backend
	esdm_backend_t *backend = (esdm_backend_t*) g_hash_table_lookup(esdm.modules->backends, "p1");  // TODO: decision component
	backend->callbacks.fragment_update(backend, fragment);
	

	esdm_dataspace_string_descriptor(fragment->dataspace);


	esdm.modules->metadata->callbacks.fragment_update(esdm.modules->metadata, fragment);


	fragment->status = ESDM_PERSISTENT;

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
esdm_status_t esdm_fragment_serialize(esdm_fragment_t *fragment, void **out)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}


/**
 * Reinstantiate fragment from serialization.
 */
esdm_fragment_t* esdm_fragment_deserialize(void *serialized_fragment)
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
esdm_dataset_t* esdm_dataset_create(esdm_container_t* container, char* name, esdm_dataspace_t* dataspace)
{
	ESDM_DEBUG(__func__);	
	esdm_dataset_t* dataset = (esdm_dataset_t*) malloc(sizeof(esdm_dataset_t));

	asprintf(&dataset->name, name);
	dataset->container = container;
	dataset->metadata = NULL;
	dataset->dataspace = dataspace;
	dataset->fragments = g_hash_table_new(g_direct_hash,  g_direct_equal);

	g_hash_table_insert(container->datasets, name, dataset);

	return dataset;
}


esdm_dataset_t* esdm_dataset_retrieve(esdm_container_t *container, const char* name)
{
	ESDM_DEBUG(__func__);	
	esdm_dataset_t* dataset = (esdm_dataset_t*) malloc(sizeof(esdm_dataset_t));

	asprintf(&dataset->name, name);
	dataset->container = container;
	dataset->metadata = NULL;
	dataset->dataspace = NULL;
	dataset->fragments = g_hash_table_new(g_direct_hash,  g_direct_equal);

	// TODO: Retrieve from MD
	// TODO: Retrieve associated Data

	return dataset;
}


esdm_status_t esdm_dataset_update(esdm_dataset_t *dataset) 
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}



esdm_status_t esdm_dataset_destroy(esdm_dataset_t *dataset) 
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}

/**
 * Make dataset persistent to storage.
 * Schedule for writing to backends.
 */
esdm_status_t esdm_dataset_commit(esdm_dataset_t *dataset)
{
	ESDM_DEBUG(__func__);	

	// print datasets of this container
	g_hash_table_foreach(dataset->fragments, print_hashtable_entry, NULL);


	// TODO: ensure callback is not NULL
	// md callback create/update container
	esdm.modules->metadata->callbacks.dataset_create(esdm.modules->metadata, dataset);


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
esdm_dataspace_t* esdm_dataspace_create(int64_t dimensions, int64_t* bounds, esdm_datatype_t datatype)
{
	ESDM_DEBUG(__func__);	
	esdm_dataspace_t* dataspace = (esdm_dataspace_t*) malloc(sizeof(esdm_dataspace_t));

	dataspace->dimensions = dimensions;
	dataspace->bounds = (int64_t*) malloc(sizeof(int64_t)*dimensions);
	dataspace->size = NULL;
	dataspace->subsize = NULL;
	dataspace->datatype = datatype;
	dataspace->subspace_of = NULL;

	// copy bounds
	memcpy(dataspace->bounds, bounds, sizeof(int64_t)*dimensions);


	for (int64_t i = 0; i < dimensions; i++) { printf("dim %d, bound=%d (%p)\n", i, bounds[i], bounds); }
	for (int64_t i = 0; i < dimensions; i++) { printf("dim %d, bound=%d (%p)\n", i, dataspace->bounds[i], dataspace->bounds); }



	printf("New dataspace: dims=%d\n", dataspace->dimensions);

	return dataspace;
}



uint8_t esdm_dataspace_overlap(esdm_dataspace_t *a, esdm_dataspace_t *b)
{
	uint8_t overlapping = 1;

	// TODO: allow comparison of spaces of different size? Alternative maybe to transform into comparable space, provided a mask or dimension index mapping
	
	if ( a->dimensions != b->dimensions )
	{
		// dimensions do not match so, we say they can not overlap
		return 0;
	}



}


/**
 * this could also be the fragment???
 *
 *
 */
esdm_dataspace_t* esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dimensions, int64_t *size, int64_t *offset)
{
	ESDM_DEBUG(__func__);	

	esdm_dataspace_t* subspace = NULL; 

	if (dimensions == dataspace->dimensions)
	{
		// replicate original space
		subspace = (esdm_dataspace_t*) malloc(sizeof(esdm_dataspace_t));
		memcpy(subspace, dataspace, sizeof(esdm_dataspace_t));

		// populate subspace members
		subspace->bounds = (int64_t*) malloc(sizeof(int64_t)*dimensions);
		subspace->subsize = (int64_t*) malloc(sizeof(int64_t)*dimensions);
		subspace->size = (int64_t*) malloc(sizeof(int64_t)*dimensions);
		subspace->offset = (int64_t*) malloc(sizeof(int64_t)*dimensions);
		subspace->subspace_of = dataspace;

		// make copies where necessary
		memcpy(subspace->bounds, dataspace->bounds, sizeof(int64_t)*dimensions);
		memcpy(subspace->subsize, size, sizeof(int64_t)*dimensions);
		memcpy(subspace->offset, offset, sizeof(int64_t)*dimensions);



		for (int64_t i = 0; i < dimensions; i++) { printf("dim %d, bounds=%d (%p)\n", i, dataspace->bounds[i], dataspace->bounds); }
		for (int64_t i = 0; i < dimensions; i++) { printf("dim %d, size=%d (%p)\n", i, size[i], size); }
	}
	else
	{
		ESDM_ERROR("Subspace dimensions do not match original space.");
	}

	return subspace;
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

esdm_status_t esdm_dataspace_serialize(esdm_dataspace_t *dataspace, void **out)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}

/**
 * Reinstantiate dataspace from serialization.
 */
esdm_dataspace_t* esdm_dataspace_deserialize(void *serialized_dataspace)
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}
