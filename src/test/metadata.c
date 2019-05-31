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

void esdm_dataset_link_metadata (esdm_dataset_t *, smd_attr_t *);

int main(){
	esdm_status ret;

	char desc[] = "{\"dataset\": \"abcdef\", \"dims\": 3}";
	char* result = NULL;

	printf("\n\n\n\nlook at here\n\n\n\n\n");

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

/*
	esdm_metadata metadata = {
		.json = "Luciana Rocha Pedro",
		.size = 30
	};
	*/

	esdm_metadata *metadata;

	metadata = (esdm_metadata *) malloc(sizeof(esdm_metadata));
	metadata->json = (char *) malloc(50*sizeof(char));

	strcpy(metadata->json, "Luciana Rocha Pedro");
	metadata->size = strlen(metadata->json);

	dataspace = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64);
	container = esdm_container_create("mycontainer", metadata);
	dataset = esdm_dataset_create(container, "mydataset", dataspace);

	// Use the smd_attr_t library to set some scientific metadata onto the object
	// We create two different attributes, one is an array and one is an integer and attach it to the dataset

	size_t len = 2;
	int a = 3;
	int *idp = &a;
	smd_attr_t *new = smd_attr_new("int", SMD_DTYPE_UINT64, &len, *idp);

  // this call registers the metadata and links it to the dataset. You are not supposed to change the metadata thereafter. it is now owned by the dataset, don`t free it either.
	//esdm_dataset_link_metadata(dataset, new);

	// this step shall write out the metadata and make it persistent
	esdm_container_commit(container);
	esdm_dataset_commit(dataset);

	// remove everything from memory
	esdm_dataset_destroy(dataset);
	esdm_container_destroy(container);


	// read back the metadata
	// open the file
	// inquire what scientific metadata exists
	// return it somehow

	ret = esdm_finalize();
	assert(ret == ESDM_SUCCESS);

	printf("OK\n");

	return 0;
}

void esdm_dataset_link_metadata (esdm_dataset_t *dataset, smd_attr_t *new) {

	char *buff; // malloc
	size_t j;

//	fragment->metadata->size += sprintf(& fragment->metadata->json[fragment->metadata->size], "{\"path\" : \"%s\"}", path_fragment);

//	sprintf(& dataset->metadata, "{\"path\" : \"%s\"}", path_fragment);
	buff = (char *)malloc(100*sizeof(char *));
	j = smd_attr_ser_json(buff, new);

}
