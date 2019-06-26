/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esdm.h>
#include <esdm-internal.h>


#define DEBUG_ENTER 		ESDM_DEBUG_COM_FMT("DATATYPES", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("DATATYPES", fmt, __VA_ARGS__)

extern esdm_instance_t esdm;


// Container //////////////////////////////////////////////////////////////////


esdm_status esdm_container_create(const char* name, esdm_container **out_container)
{
	ESDM_DEBUG(__func__);
	esdm_container* container = (esdm_container*) malloc(sizeof(esdm_container));

	container->name = strdup(name);

	container->metadata = NULL;
	container->datasets = g_hash_table_new(g_direct_hash,  g_direct_equal);
	container->status = ESDM_STATUS_DIRTY;

	*out_container = container;

	return ESDM_SUCCESS;
}


esdm_status esdm_container_retrieve(const char * name, esdm_container **out_container)
{
	ESDM_DEBUG(__func__);
	esdm_container* container = (esdm_container*) malloc(sizeof(esdm_container));

	// TODO: retrieve from MD
	// TODO: retrieve associated data

	container->name = strdup(name);

	container->metadata = NULL;
	container->datasets = g_hash_table_new(g_direct_hash,  g_direct_equal);
	container->status = ESDM_STATUS_DIRTY;

	*out_container = container;

	return ESDM_SUCCESS;
}


esdm_status esdm_container_commit(esdm_container* container)
{
	ESDM_DEBUG(__func__);

	// print datasets of this container
	esdm_print_hashtable(container->datasets);

	// TODO: ensure callback is not NULL
	// md callback create/update container
	esdm.modules->metadata_backend->callbacks.container_create(esdm.modules->metadata_backend, container);

	// Also commit uncommited datasets of this container?
	//g_hash_table_foreach (container->datasets, /* TODO: dataset commit wrapper? */ print_hashtable_entry, NULL);

	return ESDM_SUCCESS;
}


esdm_status esdm_container_destroy(esdm_container *container)
{
	ESDM_DEBUG(__func__);
	esdm_container_commit(container); // why??
	free(container);

	return ESDM_SUCCESS;
}


// Fragment ///////////////////////////////////////////////////////////////////


/**
 *	TODO: there should be a mode to auto-commit on creation?
 *
 *	How does this integrate with the scheduler? On auto-commit this merely beeing pushed to sched for dispatch?
 */

esdm_status esdm_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, void *buf, esdm_fragment_t ** out_fragment)
{
	ESDM_DEBUG(__func__);
	esdm_fragment_t* fragment = (esdm_fragment_t*) malloc(sizeof(esdm_fragment_t));

	int64_t i;
	for (i = 0; i < subspace->dimensions; i++) {
		DEBUG("dim %d, size=%d (%p)\n", i, subspace->size[i], subspace->size);
		assert(subspace->size[i] > 0);
	}

	uint64_t elements = esdm_dataspace_element_count(subspace);
	int64_t bytes = elements * esdm_sizeof(subspace->datatype);
	DEBUG("Entries in subspace: %d x %d bytes = %d bytes \n", elements, esdm_sizeof(subspace->datatype), bytes);

	fragment->metadata = malloc(ESDM_MAX_SIZE);
	fragment->metadata->json = (char*)(fragment->metadata) + sizeof(esdm_metadata);
	fragment->metadata->json[0] = 0;
	fragment->metadata->size = 0;

	fragment->dataset = dataset;
	fragment->dataspace = subspace;
	fragment->buf = buf;	// zero copy?
	fragment->elements = elements;
	fragment->bytes = bytes;
	fragment->status = ESDM_STATUS_DIRTY;

	*out_fragment = fragment;

	return ESDM_SUCCESS;
}


esdm_status esdm_dataspace_overlap_str(esdm_dataspace_t *a, char delim_c, char * str_offset, char * str_size, esdm_dataspace_t ** out_space){
	//printf("str: %s %s\n", str_size, str_offset);

	const char delim[] = {delim_c, 0};
	char * save = NULL;
	char * cur = strtok_r(str_offset, delim, & save);
	if (cur == NULL) return ESDM_ERROR;

	int64_t off[a->dimensions];
	int64_t size[a->dimensions];

	for(int d=0; d < a->dimensions; d++){
		if (cur == NULL) return ESDM_ERROR;
		off[d] = atol(cur);
		if(off[d] < 0) return ESDM_ERROR;
		//printf("o%ld,", off[d]);
		save[-1] = delim_c; // reset the overwritten text
		cur = strtok_r(NULL, delim, & save);
	}
	//printf("\n");
	if(str_size != NULL){
		cur = strtok_r(str_size, delim, & save);
	}

	for(int d=0; d < a->dimensions; d++){
		if (cur == NULL) return ESDM_ERROR;
		size[d] = atol(cur);
		if(size[d] < 0) return ESDM_ERROR;
		//printf("s%ld,", size[d]);
		if(save[0] != 0) save[-1] = delim_c;
		cur = strtok_r(NULL, delim, & save);
	}
	//printf("\n");
	if( cur != NULL){
		return ESDM_ERROR;
	}

	// dimensions match, now check for overlap
	for(int d=0; d < a->dimensions; d++){
		int o1 = a->offset[d];
		int s1 = a->size[d];

		int o2 = off[d];
		int s2 = size[d];

		//printf("%ld %ld != %ld %ld\n", o1, s1, o2, s2);
		if ( o1 + s1 <= o2 ) return ESDM_ERROR;
		if ( o2 + s2 <= o1 ) return ESDM_ERROR;
	}
	if(out_space != NULL){
		// TODO always go to the parent space
		esdm_dataspace_t * parent = a->subspace_of == NULL ? a : a->subspace_of;
		esdm_dataspace_subspace(parent, a->dimensions, size, off, out_space);
	}
	return ESDM_SUCCESS;
}


esdm_status esdm_fragment_retrieve(esdm_fragment_t *fragment)
{
	ESDM_DEBUG(__func__);

	json_t *root = load_json(fragment->metadata->json);
	json_t * elem;
	elem = json_object_get(root, "data");

	// Call backend
	esdm_backend *backend = fragment->backend;  // TODO: decision component, upon many
	backend->callbacks.fragment_retrieve(backend, fragment, elem);

	return ESDM_SUCCESS;
}


void esdm_dataspace_string_descriptor(char* string, esdm_dataspace_t *dataspace)
{
	ESDM_DEBUG(__func__);
	int pos = 0;

	int64_t dimensions = dataspace->dimensions;
	int64_t *size = dataspace->size;
	int64_t *offset = dataspace->offset;

	// offset to string
	int64_t i;
	pos += sprintf(& string[pos], "%ld", offset[0]);
	for (i = 1; i < dimensions; i++)
	{
		DEBUG("dim %d, offset=%ld (%p)\n", i, offset[i], offset);
		pos += sprintf(& string[pos], ",%ld", offset[i]);
	}

	// size to string
	pos += sprintf(& string[pos], ",%ld", size[0]);
	for (i = 1; i < dimensions; i++)
	{
		DEBUG("dim %d, size=%ld (%p)\n", i, size[i], size);
		pos += sprintf(& string[pos], ",%ld", size[i]);
	}
	DEBUG("Descriptor: %s\n", string);
}


esdm_status esdm_fragment_commit(esdm_fragment_t *f)
{
	ESDM_DEBUG(__func__);
	esdm_metadata * m = f->metadata;
	esdm_dataspace_t * d = f->dataspace;

	m->size += sprintf(& m->json[m->size], "{\"plugin\" : \"%s\", \"id\" : \"%s\", \"size\": \"", f->backend->name, f->backend->config->id);

	m->size += sprintf(& m->json[m->size], "%ld", d->size[0]);
	for(int i=1; i < d->dimensions; i++){
		m->size += sprintf(& m->json[m->size], "x%ld", d->size[i]);
	}

	m->size += sprintf(& m->json[m->size], "\", \"offset\" :\"");
	m->size += sprintf(& m->json[m->size], "%ld", d->offset[0]);
	for(int i=1; i < d->dimensions; i++){
		m->size += sprintf(& m->json[m->size], "x%ld", d->offset[i]);
	}

	m->size += sprintf(& m->json[m->size], "\", \"data\" :");

	f->backend->callbacks.fragment_update(f->backend, f);
	m->size += sprintf(& m->json[m->size], "}");

	// Announce to metadata coordinator
	esdm.modules->metadata_backend->callbacks.fragment_update(esdm.modules->metadata_backend, f);

	f->status = ESDM_STATUS_PERSISTENT;

	return ESDM_SUCCESS;
}


esdm_status esdm_fragment_destroy(esdm_fragment_t *fragment)
{
	ESDM_DEBUG(__func__);
	return ESDM_SUCCESS;
}


esdm_status esdm_fragment_serialize(esdm_fragment_t *fragment, void **out)
{
	ESDM_DEBUG(__func__);

	return ESDM_SUCCESS;
}


 esdm_status esdm_fragment_deserialize(void *serialized_fragment, esdm_fragment_t ** _out_fragment)
{
	ESDM_DEBUG(__func__);
	return ESDM_SUCCESS;
}


// Dataset ////////////////////////////////////////////////////////////////////

void esdm_dataset_dataspace_serialize_recursively_(smd_attr_t * smd, esdm_dataspace_t* dataspace){
	if(dataspace != NULL){
		smd_dtype_t * t_arr = smd_type_array(SMD_DTYPE_INT64, dataspace->dimensions);
		smd_attr_t * vars = smd_attr_new("space", t_arr, dataspace->size, 0);
		smd_attr_link(smd, vars, 0);
		esdm_dataset_dataspace_serialize_recursively_(vars, dataspace->subspace_of);

		vars = smd_attr_new("offset", t_arr, dataspace->offset, 0);
		smd_attr_link(smd, vars, 0);
	}
}

esdm_status esdm_dataset_create(esdm_container* container, const char* name, esdm_dataspace_t* dataspace,  esdm_dataset_t ** out_dataset)
{
	ESDM_DEBUG(__func__);
	esdm_dataset_t* dataset = (esdm_dataset_t*) malloc(sizeof(esdm_dataset_t));

	dataset->name = strdup(name);
	dataset->container = container;
	esdm_metadata_init_(& dataset->metadata);
	dataset->dataspace = dataspace;

	smd_dtype_t * t_arr = smd_type_array(SMD_DTYPE_INT64, dataspace->dimensions);
	smd_attr_t * vars = smd_attr_new("dims", t_arr, dataspace->size, 0);
	esdm_dataset_dataspace_serialize_recursively_(vars, dataspace->subspace_of);
	smd_attr_link(dataset->metadata->tech, vars, 0);
	vars = smd_attr_new("type", SMD_DTYPE_DTYPE, dataspace->datatype, 0);
	smd_attr_link(dataset->metadata->tech, vars, 0);

	*out_dataset = dataset;

	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_retrieve(esdm_container *container, const char * name, esdm_dataset_t **out_dataset)
{
	ESDM_DEBUG(__func__);
	esdm_dataset_t* dataset = (esdm_dataset_t*) malloc(sizeof(esdm_dataset_t));

	dataset->name = strdup(name);
	dataset->container = container;
	dataset->metadata = (esdm_metadata *) malloc(sizeof(esdm_metadata));
	dataset->dataspace = NULL;

	dataset->metadata->size = 0;
	dataset->metadata->buff_size = 0;

	esdm.modules->metadata_backend->callbacks.dataset_retrieve(esdm.modules->metadata_backend, dataset);

	// TODO: Retrieve associated Data

	*out_dataset = dataset;

	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_update(esdm_dataset_t *dataset)
{
	ESDM_DEBUG(__func__);
	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_destroy(esdm_dataset_t *dataset)
{
	ESDM_DEBUG(__func__);
//	free(dataset->name);
	free(dataset->metadata);
	dataset->metadata = NULL;
//	free(dataset->container);
//	free(dataset->dataspace);
//	free(dataset);
	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_commit(esdm_dataset_t *dataset)
{
	ESDM_DEBUG(__func__);
	dataset->metadata->size = smd_attr_ser_json(dataset->metadata->json, dataset->metadata->smd) - 1;

	// TODO: ensure callback is not NULL
	// md callback create/update container
	esdm.modules->metadata_backend->callbacks.dataset_create(esdm.modules->metadata_backend, dataset);

	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_read_metadata(esdm_dataset_t * dataset, esdm_metadata ** out_metadata)
{
	//smd_attr_t *out = smd_attr_create_from_json(dataset->metadata->json, dataset->metadata->size);
	*out_metadata = dataset->metadata;

	return ESDM_SUCCESS;
}


// Dataspace //////////////////////////////////////////////////////////////////


esdm_status esdm_dataspace_create(int64_t dimensions, int64_t* sizes, esdm_datatype_t datatype, esdm_dataspace_t ** out_dataspace)
{
	ESDM_DEBUG(__func__);
	esdm_dataspace_t* dataspace = (esdm_dataspace_t*) malloc(sizeof(esdm_dataspace_t));

	dataspace->dimensions = dimensions;
	dataspace->size = (int64_t*) malloc(sizeof(int64_t)*dimensions);
	dataspace->datatype = datatype;
	dataspace->offset = (int64_t*) malloc(sizeof(int64_t)*dimensions);
	dataspace->subspace_of = NULL;

	memcpy(dataspace->size, sizes, sizeof(int64_t)*dimensions);
	memset(dataspace->offset, 0, sizeof(int64_t)*dimensions);

	DEBUG("New dataspace: dims=%d\n", dataspace->dimensions);

	* out_dataspace = dataspace;

	return ESDM_SUCCESS;
}


uint8_t esdm_dataspace_overlap(esdm_dataspace_t *a, esdm_dataspace_t *b)
{
	// TODO: allow comparison of spaces of different size? Alternative maybe to transform into comparable space, provided a mask or dimension index mapping

	if ( a->dimensions != b->dimensions )
	{
		// dimensions do not match so, we say they can not overlap
		return 0;
	}

	return 0;
}

/**
 * this could also be the fragment???
 *
 *
 */
esdm_status esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dimensions, int64_t *size, int64_t *offset, esdm_dataspace_t **out_dataspace)
{
	ESDM_DEBUG(__func__);

	esdm_dataspace_t* subspace = NULL;

	if (dimensions == dataspace->dimensions)
	{
		// replicate original space
		subspace = (esdm_dataspace_t*) malloc(sizeof(esdm_dataspace_t));
		memcpy(subspace, dataspace, sizeof(esdm_dataspace_t));

		// populate subspace members
		subspace->size = (int64_t*) malloc(sizeof(int64_t)*dimensions);
		subspace->offset = (int64_t*) malloc(sizeof(int64_t)*dimensions);
		subspace->subspace_of = dataspace;

		// make copies where necessary
		memcpy(subspace->size, size, sizeof(int64_t)*dimensions);
		memcpy(subspace->offset, offset, sizeof(int64_t)*dimensions);

		for (int64_t i = 0; i < dimensions; i++) {
			DEBUG("dim %d, size=%ld off=%ld", i, size[i], offset[i]);
			assert(size[i] > 0);
		}
	}
	else
	{
		ESDM_ERROR("Subspace dimensions do not match original space.");
	}

	* out_dataspace = subspace;

	return ESDM_SUCCESS;
}


void esdm_dataspace_print(esdm_dataspace_t * d){
	printf("DATASPACE(size(%ld", d->size[0]);
	for (int64_t i = 1; i < d->dimensions; i++) {
		printf("x%ld", d->size[i]);
	}
	printf("),off(");
	printf("%ld", d->offset[0]);
	for (int64_t i = 1; i < d->dimensions; i++) {
		printf("x%ld", d->offset[i]);
	}
	printf("))");
}

void esdm_fragment_print(esdm_fragment_t * f){
	printf("FRAGMENT(%p,", (void *)f);
	esdm_dataspace_print(f->dataspace);
	printf(", md(\"%s\")", f->metadata->json);
	printf(")");
}


esdm_status esdm_dataspace_destroy(esdm_dataspace_t *dataspace)
{
	ESDM_DEBUG(__func__);
	return ESDM_SUCCESS;
}


esdm_status esdm_dataspace_serialize(esdm_dataspace_t *dataspace, void **out)
{
	ESDM_DEBUG(__func__);

	return ESDM_SUCCESS;
}


esdm_status esdm_dataspace_deserialize(void *serialized_dataspace, esdm_dataspace_t ** out_dataspace)
{
	ESDM_DEBUG(__func__);
	return ESDM_SUCCESS;
}


uint64_t esdm_dataspace_element_count(esdm_dataspace_t *subspace){
	assert(subspace->size != NULL);
	// calculate subspace element count
	uint64_t size = subspace->size[0];
	for (int i = 1; i < subspace->dimensions; i++)
	{
		size *= subspace->size[i];
	}
	return size;
}


uint64_t  esdm_dataspace_size(esdm_dataspace_t *dataspace){
	uint64_t size = esdm_dataspace_element_count(dataspace);
	uint64_t bytes = size*esdm_sizeof(dataspace->datatype);
	return bytes;
}


// Metadata //////////////////////////////////////////////////////////////////


esdm_status esdm_metadata_init_(esdm_metadata ** output_metadata){
	ESDM_DEBUG(__func__);

	esdm_metadata * md = (esdm_metadata*) malloc(sizeof(esdm_metadata) + 10000);
	assert(md != NULL);
	*output_metadata = md;
	md->buff_size = 10000;
	md->size = 0;
	md->json = ((char*) md) + sizeof(esdm_metadata);
	md->smd = smd_attr_new("", SMD_DTYPE_EMPTY, NULL, 0);

	md->tech = smd_attr_new("tech", SMD_DTYPE_EMPTY, NULL, 0);
	md->attr = smd_attr_new("attr", SMD_DTYPE_EMPTY, NULL, 0);
	md->special = smd_attr_new("special", SMD_DTYPE_EMPTY, NULL, 0);
	smd_attr_link(md->smd, md->tech, 0);
	smd_attr_link(md->smd, md->attr, 0);
	smd_attr_link(md->smd, md->special, 0);

	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_name_dimensions(esdm_dataset_t * dataset, int dims, char ** names){
	ESDM_DEBUG(__func__);
	// TODO check for error: int smd_find_position_by_name(const smd_attr_t * attr, const char * name);

	smd_dtype_t * t_arr = smd_type_array(SMD_DTYPE_STRING, dims);
	smd_attr_t * vars = smd_attr_new("vars", t_arr, names, 0);
	smd_attr_link(dataset->metadata->special, vars, 0);

	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_link_attribute(esdm_dataset_t * dset, smd_attr_t * attr){
	ESDM_DEBUG(__func__);

	smd_link_ret_t ret = smd_attr_link(dset->metadata->attr, attr, 0);
	return ESDM_SUCCESS;
}


esdm_status esdm_dataset_iterator(esdm_container *container, esdm_dataset_iterator_t ** iter){
	ESDM_DEBUG(__func__);

	return ESDM_SUCCESS;
}
