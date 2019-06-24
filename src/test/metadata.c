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

/*
 * Test to check the metadata APIs of ESDM.
 */

#include <stdio.h>
#include <stdlib.h>

#include <esdm.h>
#include <esdm-datatypes-internal.h>

int convert_smd_to_metadata(smd_attr_t *smd_file, esdm_metadata ** out);
//esdm_status esdm_dataset_link_metadata (esdm_dataset_t *dataset, smd_attr_t *new);


int main(){
	esdm_status ret;

	char desc[] = "{\"dataset\": \"abcdef\", \"dims\": 3}";
	char* result = NULL;

	ret = esdm_stat(desc, result);

	// Interaction with ESDM
	esdm_dataspace_t *dataspace = NULL;
	esdm_container *container = NULL;
	esdm_dataset_t *dataset = NULL;

	ret = esdm_init();
	assert( ret == ESDM_SUCCESS );


	// define dataspace
	int64_t bounds[] = {10, 20};
	// write the actual metadata

	esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, & dataspace);
	esdm_container_create("mycontainer", & container);

	// Use the smd_attr_t library to set some scientific metadata onto the object
	// We create two different attributes, one is an array and one is an integer and attach it to the dataset

	size_t len = 3;
	int idp = 5; // Can it be anything? How to get this info back?

	printf("\n\nInitial Values\n\n");

	printf("\n name = %s\n", "int");
	printf("\n len = %d\n", len);
	printf("\n idp = %d\n\n\n", idp);

	// SMD_DTYPE_UINT64 -> when are we gonna expand this?

	smd_attr_t * smd_file = smd_attr_new("int", SMD_DTYPE_UINT64, & len, idp);

	// smd_attr_print(smd_file);

  // This call registers the metadata and links it to the dataset. You are not supposed to change the metadata thereafter. it is now owned by the dataset, don`t free it either.

	esdm_metadata *metadata = NULL;
	convert_smd_to_metadata(smd_file, & metadata);

// It was like this in the whole code. I can change it later. (== ever?)

	esdm_dataset_create(container, "mydataset", dataspace, &dataset);

	// this step shall write out the metadata and make it persistent
	esdm_container_commit(container);
	esdm_dataset_commit(dataset);

	// remove everything from memory
	esdm_dataset_destroy(dataset);

//	esdm_dataset_t* esdm_dataset_retrieve(esdm_container *container, const char* name)

//	static int dataset_retrieve(esdm_backend* backend, esdm_dataset_t *dataset)

// This function doesn't exist. So I'll proceed with the more important stuff for now.

//	ret = esdm_dataset_open(container, "mydataset", & dataset);

	//esdm_dataset_retrieve_from_file(dataset);
	//esdm_dataset_read_metadata(dataset, & metadata);

	esdm_dataset_destroy(dataset);
	esdm_container_destroy(container);

	ret = esdm_finalize();
	assert(ret == ESDM_SUCCESS);

	printf("OK\n");

	return 0;
}

int convert_smd_to_metadata(smd_attr_t *smd_file, esdm_metadata ** out_metadata)
{

	char *buff;
	size_t j;

	buff = (char *) malloc(sizeof(char));
	// j = smd_attr_ser_json(buff, smd_file);

	j = smd_attr_ser_json(buff, smd_file);

	if(j > 1){ //what's a good number to guess?
		free(buff);
		buff = (char *) malloc(j*sizeof(char));
		j = smd_attr_ser_json(buff, smd_file);
	}

	esdm_metadata * metadata = (esdm_metadata *) malloc(sizeof(esdm_metadata));
	metadata->json = buff;
	metadata->smd = smd_file;
	metadata->size = j;

	printf("\nFinal\n");

	*out_metadata = metadata;

	return ESDM_SUCCESS;
}
