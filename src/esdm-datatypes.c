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
 * @brief This file implements ESDM types, and associated methods.
 */

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <esdm-internal.h>
#include <esdm.h>
#include <inttypes.h>
#include <smd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("DATATYPES", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("DATATYPES", fmt, __VA_ARGS__)

extern esdm_instance_t esdm;

// Container //////////////////////////////////////////////////////////////////

esdm_status esdm_container_create(const char *name, int allow_overwrite, esdm_container_t **oc) {
  ESDM_DEBUG(__func__);
  eassert(name);
  eassert(*name && "name must not be empty");
  eassert(oc);

  esdm_container_t * c;
  esdmI_container_init(name, & c);
  c->mode_flags = ESDM_MODE_FLAG_WRITE;

  int ret = esdm.modules->metadata_backend->callbacks.container_create(esdm.modules->metadata_backend, c, allow_overwrite);

  if(ret == ESDM_SUCCESS){
    *oc = c;
  }else{
    esdmI_container_destroy(c);
  }

  return ret;
}

bool esdm_container_dataset_exists(esdm_container_t * c, char const * name){
  eassert(c != NULL);
  eassert(name != NULL);
  esdm_datasets_t * d = & c->dsets;
  for(int i=0; i < d->count; i++){
    if (strcmp(name, d->dset[i]->name) == 0){
      return true;
    }
  }
  return false;
}

int esdm_container_dataset_count(esdm_container_t * c){
  eassert(c != NULL);
  return c->dsets.count;
}


esdm_dataset_t * esdm_container_dataset_from_array(esdm_container_t * c, int i){
  ESDM_DEBUG(__func__);
  eassert(c != NULL);
  eassert(i >= 0);
  if(i >= c->dsets.count){
    return NULL;
  }
  return c->dsets.dset[i];
}

void esdmI_container_register_dataset(esdm_container_t * c, esdm_dataset_t *dset){
	ESDM_DEBUG(__func__);
  eassert(c != NULL);
  eassert(dset != NULL);
	esdm_datasets_t * d = & c->dsets;
	if (d->buff_size == d->count){
		d->buff_size = d->buff_size * 2 + 5;
		d->dset = (esdm_dataset_t**) realloc(d->dset, sizeof(void*) * d->buff_size);
		eassert(d->dset != NULL);
	}
	d->dset[d->count] = dset;
	d->count++;

  dset->refcount++;
}

void esdmI_container_init(char const * name, esdm_container_t **out_container){
  esdm_container_t *c = (esdm_container_t *)malloc(sizeof(esdm_container_t));
  c->name = strdup(name);
  c->refcount = 1;
  c->status = ESDM_DATA_DIRTY;
  memset(& c->dsets, 0, sizeof(esdm_datasets_t));
  c->attr = smd_attr_new("Variables", SMD_DTYPE_EMPTY, NULL, 0);
  *out_container = c;
}

esdm_status esdmI_create_dataset_from_metadata(esdm_container_t *c, json_t * json, esdm_dataset_t ** out){
  json_t *elem;
  elem = json_object_get(json, "id");
  char const * id = json_string_value(elem);

  elem = json_object_get(json, "name");
  char const * name = json_string_value(elem);

  esdm_dataset_t *d;
	esdm_dataset_init(c, name, NULL, & d);
  d->id = strdup((char*) id);
  d->status = ESDM_DATA_NOT_LOADED;
  *out = d;

  return ESDM_SUCCESS;
}

esdm_status esdm_container_open_md_parse(esdm_container_t *c, char * md, int size){
	esdm_status ret;
	char * js = md;

  // first strip the attributes
  size_t parsed = smd_attr_create_from_json(js + 1, size, & c->attr);
  js += 1 + parsed;
  js[0] = '{';
  // for the rest we use JANSSON
  json_t *root = load_json(js);
  json_t *elem;
	elem = json_object_get(root, "dsets");
	if(! elem) {
		return ESDM_ERROR;
	}
	int arrsize = json_array_size(elem);
	esdm_datasets_t * d = & c->dsets;
	d->count = arrsize;
	d->buff_size = arrsize;
	d->dset = (esdm_dataset_t **) malloc(arrsize*sizeof(void*));

	for (int i = 0; i < arrsize; i++) {
		json_t * djson = json_array_get(elem, i);
		esdm_dataset_t * dset;
    ret = esdmI_create_dataset_from_metadata(c, djson, & dset);
    if (ret != ESDM_SUCCESS){
      free(d->dset);
      return ret;
    }
		d->dset[i] = dset;
	}

  json_decref(root);

  c->status = ESDM_DATA_PERSISTENT;
  c->refcount = 1;

	return ESDM_SUCCESS;
}

esdm_status esdm_container_open_md_load(esdm_container_t *c, char ** out_md, int * out_size){
	return esdm.modules->metadata_backend->callbacks.container_retrieve(esdm.modules->metadata_backend, c, out_md, out_size);
}

esdm_status esdm_container_open(char const *name, int esdm_mode_flags, esdm_container_t **out_container) {
  ESDM_DEBUG(__func__);
  eassert(out_container);
  eassert(name);
  if(!*name) {
    ESDM_LOG_FMT(ESDM_LOGLEVEL_WARNING, "%s() called with an empty name argument\n", __func__);
    return ESDM_INVALID_ARGUMENT_ERROR;
  }

  esdmI_container_init(name, out_container);
  esdm_container_t *c = *out_container;
  c->mode_flags = esdm_mode_flags;

  char * buff;
  int size;

  esdm_status ret = esdm_container_open_md_load(c, & buff, & size);
	if(ret != ESDM_SUCCESS){
		esdmI_container_destroy(c);
		return ret;
	}
	ret = esdm_container_open_md_parse(c, buff, size);
	free(buff);
	if(ret != ESDM_SUCCESS){
		esdmI_container_destroy(c);
		return ret;
	}

  return ESDM_SUCCESS;
}

static void esdmI_dataset_update_actual_size(esdm_dataset_t *d, esdm_fragment_t *frag){
  eassert(d);
  eassert(frag);
  if(! d->actual_size){
    return;
  }
  // update the actual dimension size
  for (int i = 0; i < d->dataspace->dims; i++) {
    if(d->dataspace->size[i] == 0){
      int64_t fend = frag->dataspace->size[i] + frag->dataspace->offset[i];
      if(fend > d->actual_size[i]){
        d->actual_size[i] = fend;
      }
    }
  }
}

void esdmI_datasets_reference_metadata_create(esdm_container_t *c, smd_string_stream_t * s){
  smd_string_stream_printf(s, "[");
	esdm_datasets_t * d = & c->dsets;
	for(int i=0; i < d->count; i++){
		if(i != 0){
			smd_string_stream_printf(s, ",\n");
		}
    smd_string_stream_printf(s, "{\"name\":\"%s\",\"id\":\"%s\"}", d->dset[i]->name, d->dset[i]->id);
	}
  smd_string_stream_printf(s, "]");
}

void esdmI_container_metadata_create(esdm_container_t *c, smd_string_stream_t * s){

  smd_string_stream_printf(s, "{");
  smd_attr_ser_json(s, c->attr);
	smd_string_stream_printf(s, ",\"dsets\":");
  esdmI_datasets_reference_metadata_create(c, s);
  smd_string_stream_printf(s, "}");
}

esdm_status esdm_container_commit(esdm_container_t *c) {
  ESDM_DEBUG(__func__);
  eassert(c);

  // only do work if dirty
  if(c->status != ESDM_DATA_DIRTY){
    return ESDM_SUCCESS;
  }
  c->status = ESDM_DATA_PERSISTENT;

  size_t md_size;
  smd_string_stream_t * s = smd_string_stream_create();
  esdmI_container_metadata_create(c, s);
  char * buff = smd_string_stream_close(s, & md_size);

  esdm_status ret =  esdm.modules->metadata_backend->callbacks.container_commit(esdm.modules->metadata_backend, c, buff, md_size);

  // Also commit uncommited datasets of this container: cannot do this as it depends on how we are called
  esdm_datasets_t * dsets = & c->dsets;
  for(int i = 0; i < dsets->count; i++){
   esdm_dataset_commit(dsets->dset[i]);
  }
  free(buff);
  return ret;
}

esdm_status esdm_container_close(esdm_container_t *c) {
  ESDM_DEBUG(__func__);
  eassert(c);
  if(c->status == ESDM_DATA_NOT_LOADED){
    return ESDM_SUCCESS;
  }

  eassert(c->refcount > 0);

  c->refcount--;
  if(c->refcount > 0){
    return ESDM_SUCCESS;
  }

  esdm_status ret = ESDM_SUCCESS;
  esdm_datasets_t * dsets = & c->dsets;
  for(int i = 0; i < dsets->count; i++){
    if(dsets->dset[i]->refcount != 0){	//The container always holds the information about all its datasets. However, the datasets are only loaded when they are opened, and should not remain alive without the container being alive. That is why the refcount is checked for zero here, to stop the container from being closed while there still are external references to its datasets.
      ret = ESDM_ERROR;
    }
  }
  if(ret == ESDM_SUCCESS){
    esdmI_container_destroy(c);
  }

  return ret;
}

esdm_status esdm_container_delete_attribute(esdm_container_t *c, const char *name) {
  ESDM_DEBUG(__func__);

  smd_attr_t *attr;
  esdm_status status = esdm_container_get_attributes(c, &attr);
  if (name != NULL && status == ESDM_SUCCESS){
    int pos = smd_find_position_by_name(attr, name);
    if (pos != -1){
      smd_attr_t * chld = smd_attr_get_child(attr, pos);
      smd_attr_destroy(chld);
      smd_attr_unlink_pos(attr, pos);
      return ESDM_SUCCESS;
    }
    return(ESDM_ERROR);
  }
  return(ESDM_ERROR);
}

esdm_status esdm_container_link_attribute(esdm_container_t *c, int overwrite, smd_attr_t *attr) {
  ESDM_DEBUG(__func__);
  smd_link_ret_t ret = smd_attr_link(c->attr, attr, overwrite);
  return ret == SMD_ATTR_EEXIST ? ESDM_ERROR : ESDM_SUCCESS; // I don't get it
}

esdm_status esdm_container_get_attributes(esdm_container_t *c, smd_attr_t **out_metadata) {
  eassert(c->attr != NULL);
  *out_metadata = c->attr;
  return ESDM_SUCCESS;
}

esdm_status esdmI_container_destroy(esdm_container_t *c) {
  ESDM_DEBUG(__func__);
  eassert(c);

  int ret = ESDM_SUCCESS;
  c->status = ESDM_DATA_NOT_LOADED;
  // free every dataset that is not referenced any more
  esdm_datasets_t * dsets = & c->dsets;
  for(int i = 0; i < dsets->count; i++){
    if(dsets->dset[i]->refcount <= 1){
      int lret = esdmI_dataset_destroy(dsets->dset[i]);
      if (lret != ESDM_SUCCESS){
        ret = lret;
      }
    }
  }
  if(ret == ESDM_SUCCESS){
    free(c->name);
    free(c);
  }

  return ret;
}

void esdmI_dataset_register_fragment(esdm_dataset_t *dset, esdm_fragment_t *frag){
  ESDM_DEBUG(__func__);
  esdmI_fragments_add(&dset->fragments, frag);
  dset->status = ESDM_DATA_DIRTY;
  dset->container->status = ESDM_DATA_DIRTY;
}


int64_t esdm_dataspace_get_dims(esdm_dataspace_t * d){
  eassert(d);
  return d->dims;
}

int64_t const* esdm_dataspace_get_size(esdm_dataspace_t * d){
  eassert(d);
  return d->size;
}

int64_t const* esdm_dataspace_get_offset(esdm_dataspace_t * d){
  eassert(d);
  return d->offset;
}


// Fragment ///////////////////////////////////////////////////////////////////

/**
 *	TODO: there should be a mode to auto-commit on creation?
 *
 *	How does this integrate with the scheduler? On auto-commit this merely beeing pushed to sched for dispatch?
 */

esdm_status esdmI_fragment_create(esdm_dataset_t *d, esdm_dataspace_t *sspace, void *buf, esdm_fragment_t **out_fragment) {
  eassert(d);
  ESDM_DEBUG(__func__);
  esdm_fragment_t *f = (esdm_fragment_t *)malloc(sizeof(esdm_fragment_t));

  int64_t i;
  for (i = 0; i < sspace->dims; i++) {
    DEBUG("dim %d, size=%d (%p)\n", i, sspace->size[i], sspace->size);
    eassert(sspace->size[i] > 0);
    eassert(d->dataspace->size[i] == 0 || sspace->size[i] + sspace->offset[i] <= d->dataspace->size[i]);
  }

  uint64_t elements = esdm_dataspace_element_count(sspace);
  int64_t bytes = elements * esdm_sizeof(sspace->type);
  DEBUG("Entries in sspace: %d x %d bytes = %d bytes \n", elements, esdm_sizeof(sspace->type), bytes);

  f->id = NULL;
  f->backend_md  = NULL;
  f->dataset = d;
  f->dataspace = sspace;
  f->buf = buf; // zero copy?
  f->elements = elements;
  f->bytes = bytes;
	f->status = ESDM_DATA_NOT_LOADED;
  f->backend = NULL;

	esdmI_dataset_register_fragment(d, f);
  esdmI_dataset_update_actual_size(d, f);

  *out_fragment = f;

  return ESDM_SUCCESS;
}

esdm_status esdm_fragment_retrieve(esdm_fragment_t *fragment) {
  ESDM_DEBUG(__func__);
  // Call backend
  esdm_backend_t *backend = fragment->backend;
  int ret = backend->callbacks.fragment_retrieve(backend, fragment);
  if(ret == ESDM_SUCCESS){
    fragment->status = ESDM_DATA_PERSISTENT;
  }

  return ret;
}


void esdm_fragment_metadata_create(esdm_fragment_t *f, smd_string_stream_t * stream){
  eassert(f != NULL);
	esdm_dataspace_t *d = f->dataspace;
  char const * pid = f->backend->config->id;
  eassert(f->id != NULL);
  eassert(pid != NULL);

  smd_string_stream_printf(stream, "{\"id\":\"%s\",\"pid\":\"%s\",\"size\":[", f->id, pid);
  smd_string_stream_printf(stream, "%ld", d->size[0]);
  for (int i = 1; i < d->dims; i++) {
    smd_string_stream_printf(stream, ",%ld", d->size[i]);
  }

  smd_string_stream_printf(stream, "],\"offset\":[");
  smd_string_stream_printf(stream, "%ld", d->offset[0]);
  for (int i = 1; i < d->dims; i++) {
    smd_string_stream_printf(stream, ",%ld", d->offset[i]);
  }
  if(d->stride) {
    smd_string_stream_printf(stream, "],\"stride\":[");
    smd_string_stream_printf(stream, "%ld", d->stride[0]);
    for (int i = 1; i < d->dims; i++) {
      smd_string_stream_printf(stream, ",%ld", d->stride[i]);
    }
  }
  if(f->backend->callbacks.fragment_metadata_create){
    smd_string_stream_printf(stream, "],\"backend\":");
    f->backend->callbacks.fragment_metadata_create(f->backend, f, stream);
    smd_string_stream_printf(stream, "}");
  }else{
    smd_string_stream_printf(stream, "]}");
  }
}

esdm_status esdm_fragment_commit(esdm_fragment_t *f) {
  ESDM_DEBUG(__func__);
  eassert(f && "fragment argument must not be NULL");

	int ret = f->backend->callbacks.fragment_update(f->backend, f);
  if(ret == ESDM_SUCCESS) {
    f->status = ESDM_DATA_PERSISTENT;
  }

  return ret;
}

esdm_status esdm_container_delete(esdm_container_t *c){
  ESDM_DEBUG(__func__);
  eassert(c);
  eassert(c->status != ESDM_DATA_DELETED);
  int ret = ESDM_SUCCESS;
  int status;
  for(int i=0; i < c->dsets.count; i++){
    esdm_dataset_t * ds = c->dsets.dset[i];
    status = esdm_dataset_delete(ds);
    if(status != ESDM_SUCCESS){
      if(i == 0){
        return status;
      }
      ret = status;
    }
  }
  c->status = ESDM_DATA_DELETED;
  c->dsets.count = 0;

  status = esdm.modules->metadata_backend->callbacks.container_remove(esdm.modules->metadata_backend, c);
  if(status != ESDM_SUCCESS){
    ret = status;
  }
  esdm_container_close(c);
  return ret;
}

esdm_status esdm_dataset_delete(esdm_dataset_t *d){
  ESDM_DEBUG(__func__);
  eassert(d);
  eassert(d->status != ESDM_DATA_DELETED);
  int ret = ESDM_SUCCESS;
  int status;

  if(d->status == ESDM_DATA_NOT_LOADED){
    ret = esdm_dataset_ref(d);
    if(ret != ESDM_SUCCESS){
      return ret;
    }
  }
  // TODO check usage of dataset
  int64_t fragmentCount;
  esdm_fragment_t** fragments = esdmI_fragments_list(&d->fragments, &fragmentCount);
  for(int i=0; i < fragmentCount; i++){
    esdm_fragment_t * frag = fragments[i];
    status = frag->backend->callbacks.fragment_delete(frag->backend, frag);
    if(status != ESDM_SUCCESS){
      if(i == 0){ //FIXME: This early return means that the caller does not know anything about the state of the dataset in case of an error.
                  //       Ideally, we should perform the action transactionally (either full success or no change at all),
                  //       or we should try our best to delete as many fragments as possible in case of an error.
        return status;
      }
      ret = status;
    }
    esdm_fragment_destroy(frag);
  }
  status = esdmI_fragments_destruct(&d->fragments);
  if(status != ESDM_SUCCESS) ret = status;

  d->status = ESDM_DATA_DELETED;
  status = esdm.modules->metadata_backend->callbacks.dataset_remove(esdm.modules->metadata_backend, d);
  if(status != ESDM_SUCCESS){
    ret = status;
  }
  esdm_dataset_close(d);
  return ret;
}

esdm_status esdm_fragment_destroy(esdm_fragment_t *frag) {
  ESDM_DEBUG(__func__);
  eassert(frag);
  if(frag->backend_md){
    eassert(frag->backend->callbacks.fragment_metadata_free);
    frag->backend->callbacks.fragment_metadata_free(frag->backend, frag->backend_md);
  }
  if(frag->id){
    free(frag->id);
  }
  if(frag->dataspace){
    esdm_dataspace_destroy(frag->dataspace);
  }
  if(frag->status == ESDM_DATA_PERSISTENT || frag->status == ESDM_DATA_NOT_LOADED){
    free(frag);
  }else{
    ESDM_LOG_FMT(ESDM_LOGLEVEL_WARNING, "Fragment not synchronized attempting to destroy it -- this is a memory leak.", "");
    return ESDM_ERROR;
  }

  return ESDM_SUCCESS;
}


// Dataset ////////////////////////////////////////////////////////////////////


void esdm_dataset_init(esdm_container_t *c, const char *name, esdm_dataspace_t *dspace, esdm_dataset_t **out_dataset){
  esdm_dataset_t *d = (esdm_dataset_t *)malloc(sizeof(esdm_dataset_t));

  d->dims_dset_id = NULL;
  d->name = strdup(name);
  d->id = NULL; // to be filled by the metadata backend
  d->fill_value = NULL;
  d->refcount = 0;
  d->container = c;
  d->dataspace = dspace;
  d->actual_size = NULL;
  if(dspace){
    // check for unlimited dims
    for(int i=0; i < dspace->dims; i++){
      if(dspace->size[i] == 0){
        d->actual_size = malloc(sizeof(*d->actual_size) * dspace->dims);
        memcpy(d->actual_size, dspace->size, sizeof(*d->actual_size) * dspace->dims);
        break;
      }
    }
  }
  d->status = ESDM_DATA_DIRTY;
  esdmI_fragments_construct(&d->fragments);
  d->attr = smd_attr_new("Variables", SMD_DTYPE_EMPTY, NULL, 0);

  *out_dataset = d;
}

esdm_status esdm_dataset_create(esdm_container_t *c, const char *name, esdm_dataspace_t *dspace, esdm_dataset_t **out_dataset) {
  ESDM_DEBUG(__func__);
  eassert(c);
  eassert(name);
  eassert(*name && "name must not be empty");
  eassert(dspace);
  eassert(out_dataset);


  if(! ea_is_valid_dataset_name(name)){
    return ESDM_ERROR;
  }

  if(esdm_container_dataset_exists(c, name)){
    return ESDM_ERROR;
  }
  esdm_dataset_t *dset;
  esdm_dataset_init(c, name, dspace, & dset);
  dset->mode_flags = ESDM_MODE_FLAG_WRITE;

  esdm_status status = esdm.modules->metadata_backend->callbacks.dataset_create(esdm.modules->metadata_backend, dset);
  if(status != ESDM_SUCCESS){
    esdmI_dataset_destroy(dset);
    return status;
  }

  esdmI_container_register_dataset(c, dset);
  c->status = ESDM_DATA_DIRTY;
  *out_dataset = dset;

  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_open_md_load(esdm_dataset_t *dset, char ** out_md, int * out_size){
  eassert(dset != NULL);
  eassert(out_md != NULL);
  eassert(out_size != NULL);

	return esdm.modules->metadata_backend->callbacks.dataset_retrieve(esdm.modules->metadata_backend, dset, out_md, out_size);
}

esdm_backend_t * esdmI_get_backend(char const * plugin_id){
    eassert(plugin_id);

    // find the backend for the fragment
    esdm_backend_t *backend_to_use = NULL;
    for (int x = 0; x < esdm.modules->data_backend_count; x++) {
      esdm_backend_t *b_tmp = esdm.modules->data_backends[x];
      if (strcmp(b_tmp->config->id, plugin_id) == 0) {
        DEBUG("Found plugin %s", plugin_id);
        backend_to_use = b_tmp;
        break;
      }
    }
    if (backend_to_use == NULL) {
      ESDM_ERROR_FMT("Error no backend found for ID: %s", plugin_id);
    }
	return backend_to_use;
}

esdm_status esdmI_create_fragment_from_metadata(esdm_dataset_t *dset, json_t * json, esdm_fragment_t ** out) {
  int64_t dims = dset->dataspace->dims;
  if(dims == 0) dims = 1;

  int ret;
  esdm_fragment_t *f;
  f = malloc(sizeof(esdm_fragment_t));

  json_t *elem;
  elem = json_object_get(json, "pid");
  const char *plugin_id = json_string_value(elem);
  f->backend = esdmI_get_backend(plugin_id);
  eassert(f->backend);

  elem = json_object_get(json, "id");
  char const  * id = json_string_value(elem);
  f->id = strdup(id);

  elem = json_object_get(json, "offset");
  int cnt = json_array_size(elem);
  if(cnt != dims){
    return ESDM_ERROR;
  }
  int64_t offset[dims];
  for(int i=0; i < dims; i++){
    offset[i] = json_integer_value(json_array_get(elem, i));
  }

  elem = json_object_get(json, "size");
  int cnt2 = json_array_size(elem);
  if(cnt2 != dims){
    return ESDM_ERROR;
  }
  int64_t size[cnt2];
  for(int i=0; i < cnt2; i++){
    size[i] = json_integer_value(json_array_get(elem, i));
  }
  esdm_dataspace_t * space;

  elem = json_object_get(json, "stride");
  bool haveStride = (elem != NULL);
  int64_t stride[dims];
  if(haveStride) {
    int cnt = json_array_size(elem);
    if(cnt != dims){
      return ESDM_ERROR;
    }
    for(int i=0; i < dims; i++){
      stride[i] = json_integer_value(json_array_get(elem, i));
    }
  }

  ret = esdm_dataspace_subspace(dset->dataspace, dims, size, offset, & space);
  eassert(ret == ESDM_SUCCESS);

  if(haveStride) {
    esdm_dataspace_set_stride(space, stride);
  }

  uint64_t elements = esdm_dataspace_element_count(space);
  int64_t bytes = elements * esdm_sizeof(space->type);

  f->dataset = dset;
  f->dataspace = space;
  f->buf = NULL;
  f->elements = elements;
  f->bytes = bytes;
	f->status = ESDM_DATA_NOT_LOADED;

  // deserialize module specific options
  if(f->backend->callbacks.fragment_metadata_load){
    elem = json_object_get(json, "backend");
    f->backend_md = f->backend->callbacks.fragment_metadata_load(f->backend, f, elem);
  }else{
    f->backend_md = NULL;
  }

  *out = f;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataspace_set_stride(esdm_dataspace_t* space, int64_t* stride){
  eassert(space);
  int dims = space->dims;

  if(! space->stride){
    space->stride = malloc(dims * sizeof(int64_t));
  }
  memcpy(space->stride, stride, dims * sizeof(int64_t));

  return ESDM_SUCCESS;
}

esdm_status esdm_dataspace_copyDatalayout(esdm_dataspace_t* space, esdm_dataspace_t* source) {
  eassert(space);
  eassert(source);
  eassert(space->dims == source->dims);

  //get rid of old stride array
  free(space->stride);
  space->stride = NULL;

  //check whether we actually need a stride array
  if(!source->stride) {
    bool haveMismatch = false;
    for(int64_t i = 1; i < space->dims && !haveMismatch; i++) { //don't check first dimension size, it's irrelevant for the effective strides
      haveMismatch = space->size[i] != source->size[i];
    }
    if(!haveMismatch) return ESDM_SUCCESS;  //no explicit stride in source and identical dim sizes -> implicit stride matches -> no need to set explicit stride
  }

  //copy the stride info from the source
  space->stride = malloc(space->dims*sizeof(*space->stride));
  esdm_dataspace_getEffectiveStride(source, space->stride);

  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_open_md_parse(esdm_dataset_t *d, char * md, int size){
  esdm_status ret;
  char * js = md;

  // first strip the attributes
  size_t parsed = smd_attr_create_from_json(js + 1, size, & d->attr);
  js += 1 + parsed;
  if(strncmp(js + 1, "\"fill-value\"", 12) == 0){
    parsed = smd_attr_create_from_json(js + 1, size, & d->fill_value);
    js += 1 + parsed;
  }
  js[0] = '{';
  // for the rest we use JANSSON
  json_t *root = load_json(js);
  json_t *elem;
  elem = json_object_get(root, "typ");
  char *str = (char *)json_string_value(elem);
  smd_dtype_t *type = smd_type_from_ser(str);
  if (type == NULL) {
    DEBUG("Cannot parse type: %s", str);
    return ESDM_ERROR;
  }
  elem = json_object_get(root, "id");
  d->id = strdup(json_string_value(elem));
  elem = json_object_get(root, "dims");
  int dims = json_integer_value(elem);
  elem = json_object_get(root, "size");
  int64_t sizes[dims];
  size_t arrsize;
  bool has_ulim_dim = FALSE; // if true, then we must reconstruct the domain!
  if(elem){
    arrsize = json_array_size(elem);
    if (dims != arrsize) {
      json_decref(root);
      return ESDM_ERROR;
    }
    for (int i = 0; i < dims; i++) {
      sizes[i] = json_integer_value(json_array_get(elem, i));
      if(sizes[i] == 0){
        has_ulim_dim = TRUE;
      }
    }
  }
  ret = esdm_dataspace_create(dims, sizes, type, &d->dataspace);
  if (ret != ESDM_SUCCESS) {
    json_decref(root);
    return ret;
  }
  if(has_ulim_dim){
    d->actual_size = malloc(sizeof(*d->actual_size) * dims);
    memcpy(d->actual_size, sizes, sizeof(*d->actual_size) * dims);
  }
  elem = json_object_get(root, "dims_dset_id");
  if (elem){
    arrsize = json_array_size(elem);
    if (dims != arrsize) {
      return ESDM_ERROR;
    }
    char *strs[dims];
    for (int i = 0; i < dims; i++) {
      strs[i] = (char *)json_string_value(json_array_get(elem, i));
    }
    esdm_dataset_name_dims(d, strs);
  }

  elem = json_object_get(root, "fragments");
  if(! elem) {
    json_decref(root);
    return ESDM_ERROR;
  }
  arrsize = json_array_size(elem);
  for (int i = 0; i < arrsize; i++) {
    json_t * fjson = json_array_get(elem, i);
    esdm_fragment_t * frag;
    int status = esdmI_create_fragment_from_metadata(d, fjson, & frag);
    if (status != ESDM_SUCCESS) {
      ret = status;
    } else {
      esdmI_fragments_add(&d->fragments, frag);
      esdmI_dataset_update_actual_size(d, frag);
    }
  }
  json_decref(root);

  d->status = ESDM_DATA_PERSISTENT;

  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_ref(esdm_dataset_t * d){
  ESDM_DEBUG(__func__);
  if(d->status != ESDM_DATA_NOT_LOADED){
    d->refcount++;
    return ESDM_SUCCESS;
  }

	char * buff;
  int size;
  esdm_status ret = esdm_dataset_open_md_load(d, & buff, & size);
	if(ret != ESDM_SUCCESS){
		return ret;
	}
	ret = esdm_dataset_open_md_parse(d, buff, size);
	free(buff);
	if(ret != ESDM_SUCCESS){
		return ret;
	}

  d->refcount++;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_by_name(esdm_container_t *c, const char *name, int esdm_mode_flags, esdm_dataset_t **out_dataset){
  ESDM_DEBUG(__func__);
  eassert(c);
  eassert(name);
  eassert(out_dataset);
  if(!*name) {
    ESDM_LOG_FMT(ESDM_LOGLEVEL_WARNING, "%s() called with an empty name argument\n", __func__);
    return ESDM_INVALID_ARGUMENT_ERROR;
  }

  esdm_dataset_t *d = NULL;
  esdm_datasets_t * dsets = & c->dsets;
  for(int i=0; i < dsets->count; i++ ){
    if(strcmp(dsets->dset[i]->name, name) == 0){
      d = dsets->dset[i];
      break;
    }
  }
  if(! d){
    return ESDM_ERROR;
  }
  d->mode_flags = esdm_mode_flags;
  *out_dataset = d;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_open(esdm_container_t *c, const char *name, int esdm_mode_flags, esdm_dataset_t **out_dataset) {
  esdm_status ret;
  esdm_dataset_t *d = NULL;
  ret = esdm_dataset_by_name(c, name, esdm_mode_flags, &d);
  if (ret != ESDM_SUCCESS){
    return ret;
  }

  ret = esdm_dataset_ref(d);
  if (ret == ESDM_SUCCESS){
    *out_dataset = d;
  }
  return ret;
}

void esdmI_dataset_metadata_create(esdm_dataset_t *d, smd_string_stream_t*s){
  eassert(d->dataspace != NULL);

  smd_string_stream_printf(s, "{");
  smd_attr_ser_json(s, d->attr);
  if(d->fill_value){
    smd_string_stream_printf(s, ",");
    smd_attr_ser_json(s, d->fill_value);
  }
  smd_string_stream_printf(s, ",\"id\":\"%s\"", d->id);
  smd_string_stream_printf(s, ",\"typ\":\"");
  smd_type_ser(s, d->dataspace->type);
  if(d->dataspace->dims != 0){
    smd_string_stream_printf(s,"\",\"dims\":%" PRId64 ",\"size\":[%" PRId64, d->dataspace->dims, d->dataspace->size[0]);
    for (int i = 1; i < d->dataspace->dims; i++) {
      smd_string_stream_printf(s, ",%" PRId64, d->dataspace->size[i]);
    }
    smd_string_stream_printf(s, "]");
  }else{
    smd_string_stream_printf(s,"\",\"dims\":0");
  }
  if (d->dims_dset_id != NULL) {
    smd_string_stream_printf(s, ",\"dims_dset_id\":[");
    if(d->dataspace->dims > 0){
      smd_string_stream_printf(s, "\"%s\"", d->dims_dset_id[0]);
      for (int i = 1; i < d->dataspace->dims; i++) {
        smd_string_stream_printf(s, ",\"%s\"", d->dims_dset_id[i]);
      }
    }
    smd_string_stream_printf(s, "]");
  }
	smd_string_stream_printf(s, ",\"fragments\":");
  esdmI_fragments_metadata_create(&d->fragments, s);
  smd_string_stream_printf(s, "}");
}

esdm_status esdm_dataset_commit(esdm_dataset_t *d) {
  ESDM_DEBUG(__func__);
  eassert(d);

  // only do work if dirty
  if(d->status != ESDM_DATA_DIRTY){
    return ESDM_SUCCESS;
  }
  d->status = ESDM_DATA_PERSISTENT;

  size_t md_size;
  smd_string_stream_t* stream = smd_string_stream_create();
  esdmI_dataset_metadata_create(d, stream);
  char* buff = smd_string_stream_close(stream, & md_size);
  // TODO commit each uncommited fragment

  // md callback create/update container
  esdm_status ret = esdm.modules->metadata_backend->callbacks.dataset_commit(esdm.modules->metadata_backend, d, buff, md_size);
  free(buff);

  return ret;
}

esdm_status esdm_dataset_update(esdm_dataset_t *dataset) {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_close(esdm_dataset_t *dset) {
  ESDM_DEBUG(__func__);
  eassert(dset);
  eassert(dset->status != ESDM_DATA_NOT_LOADED);
  eassert(dset->refcount > 0);

  dset->refcount--;
  if(dset->refcount){
    return ESDM_SUCCESS;
  }
  if(dset->status == ESDM_DATA_DIRTY){
    // needs to be synchronized, though
    return ESDM_SUCCESS;
  }

  dset->status = ESDM_DATA_NOT_LOADED;

  smd_attr_destroy(dset->attr);

  return esdmI_fragments_destruct(&dset->fragments);
}

esdm_status esdmI_dataset_destroy(esdm_dataset_t *dset) {
  ESDM_DEBUG(__func__);
  eassert(dset);

  if(dset->status != ESDM_DATA_NOT_LOADED){
    // loaded to some extend
    esdm_status ret = esdmI_fragments_destruct(&dset->fragments);

    smd_attr_destroy(dset->attr); // maybe unref?

    // free dataset only if all fragments can be destroyed/are not longer in use
    if (ret != ESDM_SUCCESS){
      return ret;
    }
  }
  free(dset->name);

  if (dset->dims_dset_id) {
    free(dset->dims_dset_id);
  }
  if(dset->actual_size){
    free(dset->actual_size);
  }

  if(dset->fill_value){
    smd_attr_destroy(dset->fill_value);
  }
  free(dset);
  return ESDM_SUCCESS;
}

// not tested yet

esdm_status esdm_dataset_delete_attribute(esdm_dataset_t *dataset, const char *name){
  ESDM_DEBUG(__func__);

  eassert(name);

  smd_attr_t *attr;
  esdm_status status = esdm_dataset_get_attributes(dataset, &attr);
  if (status == ESDM_SUCCESS){
    int pos = smd_find_position_by_name(attr, name);
    if (pos != -1){
      smd_attr_t * chld = smd_attr_get_child(attr, pos);
      smd_attr_destroy(chld);
      smd_attr_unlink_pos(attr, pos);
      return ESDM_SUCCESS;
    }
    return(ESDM_ERROR);
  }
  return(ESDM_ERROR);
}

esdm_status esdm_dataset_get_attributes(esdm_dataset_t *dataset, smd_attr_t **out_metadata) {
  ESDM_DEBUG(__func__);
  eassert(dataset->attr != NULL);
  *out_metadata = dataset->attr;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_rename(esdm_dataset_t *d, const char *name) {
  ESDM_DEBUG(__func__);
  eassert(name != NULL);
  eassert(d != NULL);

  int count = esdm_container_dataset_count(d->container);
  for(int i=0; i<count; i++){
    esdm_dataset_t *dset;
    dset = esdm_container_dataset_from_array(d->container, i);
    if (strcmp(dset->name, name) == 0){
      return ESDM_ERROR;
    }
  }

  free(d->name);
  d->name = strdup(name);
  d->container->status = ESDM_DATA_DIRTY;
  return ESDM_SUCCESS;
}

// Dataspace //////////////////////////////////////////////////////////////////

esdm_status esdm_dataspace_create(int64_t dims, int64_t *sizes, esdm_type_t type, esdm_dataspace_t **out_dataspace) {
  ESDM_DEBUG(__func__);
  eassert(dims >= 0);
  eassert(sizes);
  eassert(out_dataspace);

  esdm_dataspace_t *dataspace = (esdm_dataspace_t *)malloc(sizeof(esdm_dataspace_t));

  dataspace->dims = dims;
  if(dims == 0){
    dims = 1;
  }
  dataspace->size = (int64_t *)malloc(sizeof(int64_t) * dims);
  dataspace->offset = (int64_t *)malloc(sizeof(int64_t) * dims);

  memcpy(dataspace->size, sizes, sizeof(int64_t) * dims);
  memset(dataspace->offset, 0, sizeof(int64_t) * dims);

  dataspace->type = type;
  dataspace->stride = NULL;
  DEBUG("New dataspace: dims=%d\n", dataspace->dims);

  *out_dataspace = dataspace;

  return ESDM_SUCCESS;
}

esdm_status esdmI_dataspace_createFromHypercube(esdmI_hypercube_t* extends, esdm_type_t type, esdm_dataspace_t** out_space) {
  ESDM_DEBUG(__func__);
  eassert(extends);
  eassert(out_space);

  int64_t dimensions = esdmI_hypercube_dimensions(extends);
  eassert(dimensions >= 0);

  esdm_dataspace_t* result = malloc(sizeof(*result));
  *result = (esdm_dataspace_t){
    .type = type,
    .dims = dimensions,
    .size = malloc(dimensions*sizeof(*result->size)),
    .offset = malloc(dimensions*sizeof(*result->offset)),
    .stride = NULL
  };
  esdmI_hypercube_getOffsetAndSize(extends, result->offset, result->size);

  *out_space = result;
  return ESDM_SUCCESS;
}

esdm_status esdmI_dataspace_getExtends(esdm_dataspace_t* space, esdmI_hypercube_t** out_extends) {
  ESDM_DEBUG(__func__);
  eassert(space);
  eassert(out_extends);

  *out_extends = esdmI_hypercube_make(space->dims, space->offset, space->size);
  return ESDM_SUCCESS;
}

esdm_status esdmI_dataspace_setExtends(esdm_dataspace_t* space, esdmI_hypercube_t* extends) {
  ESDM_DEBUG(__func__);
  eassert(space);
  eassert(extends);
  eassert(space->dims == esdmI_hypercube_dimensions(extends));

  esdmI_hypercube_getOffsetAndSize(extends, space->offset, space->size);
  return ESDM_SUCCESS;
}

/**
 * TODO: remove dims parameter for good
 */
esdm_status esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dims, int64_t *size, int64_t *offset, esdm_dataspace_t **out_dataspace) {
  ESDM_DEBUG(__func__);
  eassert(dataspace);
  eassert(!dims || size);
  eassert(!dims || offset);
  eassert(out_dataspace);

  // check for any inconsistencies between the given subspace and the dataspace
  esdm_status status = ESDM_SUCCESS;
  if(dims == dataspace->dims) {
    for (int64_t i = 0; i < dims; i++) {
      if(size[i] <= 0) {
        ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "invalid size argument to `%s()` detected: `size[%"PRId64"]` is not positive (%"PRId64")\n", __func__, i, size[i]);
        status = ESDM_INVALID_ARGUMENT_ERROR;
      }
      if(offset[i] < 0) {
        ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "invalid offset argument to `%s()` detected: `offset[%"PRId64"]` is negative (%"PRId64")\n", __func__, i, offset[i]);
        status = ESDM_INVALID_ARGUMENT_ERROR;
      }
      if(offset[i] < dataspace->offset[i]) {
        ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "invalid arguments to `%s()` detected: `offset[%"PRId64"] = %"PRId64"` is outside of the valid range for the dataspaces' dimension (offset %"PRId64", size %"PRId64")\n", __func__, i, offset[i], dataspace->offset[i], dataspace->size[i]);
        status = ESDM_INVALID_ARGUMENT_ERROR;
      }
      if(dataspace->size[i] != 0 && (offset[i] + size[i] > dataspace->offset[i] + dataspace->size[i])) {
        ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "invalid arguments to `%s()` detected: `offset[%"PRId64"] + size[%"PRId64"] = %"PRId64" + %"PRId64" = %"PRId64"` is outside of the valid range for the dataspaces' dimension (offset %"PRId64", size %"PRId64")\n", __func__, i, i, offset[i], size[i], offset[i] + size[i], dataspace->offset[i], dataspace->size[i]);
        status = ESDM_INVALID_ARGUMENT_ERROR;
      }
    }
  }

  // perform the actual operation
  if(status == ESDM_SUCCESS) {
    // replicate original space
    esdm_dataspace_t *subspace = (esdm_dataspace_t *)malloc(sizeof(esdm_dataspace_t));
    memcpy(subspace, dataspace, sizeof(esdm_dataspace_t));

    // populate subspace members
    subspace->dims = dims;
    subspace->size = (int64_t *)malloc(sizeof(int64_t) * dims);
    subspace->offset = (int64_t *)malloc(sizeof(int64_t) * dims);
    subspace->stride = NULL;
    subspace->type = dataspace->type;
    smd_type_ref(subspace->type);

    // make copies where necessary
    memcpy(subspace->size, size, sizeof(int64_t) * dims);
    memcpy(subspace->offset, offset, sizeof(int64_t) * dims);

    *out_dataspace = subspace;
  }

  return status;
}

esdm_status esdm_dataspace_makeContiguous(esdm_dataspace_t *dataspace, esdm_dataspace_t **out_dataspace) {
  ESDM_DEBUG(__func__);
  eassert(dataspace);

  return esdm_dataspace_subspace(dataspace, dataspace->dims, dataspace->size, dataspace->offset, out_dataspace);
}

void esdm_dataspace_print(esdm_dataspace_t *d) {
  printf("DATASPACE(size(%ld", d->size[0]);
  for (int64_t i = 1; i < d->dims; i++) {
    printf("x%ld", d->size[i]);
  }
  printf("),off(");
  printf("%ld", d->offset[0]);
  for (int64_t i = 1; i < d->dims; i++) {
    printf("x%ld", d->offset[i]);
  }
  if(d->stride) {
    printf("),stride(");
    printf("%ld", d->stride[0]);
    for (int64_t i = 1; i < d->dims; i++) {
      printf(", %ld", d->stride[i]);
    }
  } else {
    printf("),stride(contiguous C order");
  }
  printf("))");
}

void esdm_dataspace_getEffectiveStride(esdm_dataspace_t* space, int64_t* out_stride) {
  if(space->stride) {
    memcpy(out_stride, space->stride, space->dims*sizeof(*space->stride));
  } else {
    int64_t curSize = 1;
    for(int64_t i = space->dims; i--; curSize *= space->size[i]) {
      out_stride[i] = curSize;
    }
  }
}

int64_t esdm_dataspace_elementOffset(esdm_dataspace_t* space, int64_t* coords) {
  int64_t offset = 0;
  for(int64_t i = space->dims, size = 1; i--; size *= space->size[i]) {
    int64_t curStride = space->stride ? space->stride[i] : size;
    offset += (coords[i] - space->offset[i])*curStride;
  }
  return offset * esdm_sizeof(space->type);
}

void esdm_fragment_print(esdm_fragment_t *f) {
  printf("FRAGMENT(%p,", (void *)f);
  esdm_dataspace_print(f->dataspace);
  printf(")");
}

esdm_status esdm_dataspace_destroy(esdm_dataspace_t *d) {
  ESDM_DEBUG(__func__);
  eassert(d);
  free(d->offset);
  free(d->size);
  if(d->stride){
    free(d->stride);
  }
  free(d);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataspace_serialize(esdm_dataspace_t *dataspace, void **out) {
  ESDM_DEBUG(__func__);

  return ESDM_SUCCESS;
}

esdm_status esdm_dataspace_deserialize(void *serialized_dataspace, esdm_dataspace_t **out_dataspace) {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}

uint64_t esdm_dataspace_element_count(esdm_dataspace_t *subspace) {
  eassert(subspace->size != NULL);
  // calculate subspace element count
  uint64_t size = subspace->size[0];
  for (int i = 1; i < subspace->dims; i++) {
    size *= subspace->size[i];
  }
  return size;
}

uint64_t esdm_dataspace_size(esdm_dataspace_t *dataspace) {
  uint64_t size = esdm_dataspace_element_count(dataspace);
  uint64_t bytes = size * esdm_sizeof(dataspace->type);
  return bytes;
}

// Metadata //////////////////////////////////////////////////////////////////

esdm_status esdm_dataset_name_dims(esdm_dataset_t *d, char **names) {
  ESDM_DEBUG(__func__);
  eassert(d != NULL);
  eassert(names != NULL);
  int dims = d->dataspace->dims;
  int size = 0;
  // compute size and check that varname is conform
  for (int i = 0; i < dims; i++) {
    size += strlen(names[i]) + 1;
    if (!ea_is_valid_dataset_name(names[i])) {
      return ESDM_ERROR;
    }
  }
  void * old = d->dims_dset_id;
  d->dims_dset_id = (char **)malloc(dims * sizeof(void *) + size);
  char *posVar = (char *)d->dims_dset_id + dims * sizeof(void *);
  for (int i = 0; i < dims; i++) {
    d->dims_dset_id[i] = posVar;
    strcpy(posVar, names[i]);
    posVar += 1 + strlen(names[i]);
  }
  if (old != NULL) {
    free(old);
  }

  d->status = ESDM_DATA_DIRTY;
  d->container->status = ESDM_DATA_DIRTY;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_rename_dim(esdm_dataset_t *d, char const *name, int i){
  eassert(i >= 0);
  eassert(i < d->dataspace->dims );
  char ** names = d->dims_dset_id;
  names[i] = (char*) name;

  return esdm_dataset_name_dims(d, names);
}

void esdm_dataset_set_status_dirty(esdm_dataset_t * d){
  eassert(d->status == ESDM_DATA_DIRTY || d->status == ESDM_DATA_PERSISTENT);
  d->status = ESDM_DATA_DIRTY;
}

void esdm_container_set_status_dirty(esdm_container_t * c){
  eassert(c->status == ESDM_DATA_DIRTY || c->status == ESDM_DATA_PERSISTENT);
  c->status = ESDM_DATA_DIRTY;
}

esdm_status esdm_dataset_get_name_dims(esdm_dataset_t *d, char const *const **out_names) {
  eassert(d != NULL);
  eassert(out_names != NULL);
  *out_names = (char const *const *)d->dims_dset_id;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_link_attribute(esdm_dataset_t *dset, int overwrite, smd_attr_t *attr) {
  ESDM_DEBUG(__func__);
  smd_link_ret_t ret = smd_attr_link(dset->attr, attr, overwrite);
  return ret == SMD_ATTR_EEXIST ? ESDM_ERROR : ESDM_SUCCESS;
}

esdm_status esdm_dataset_iterator(esdm_container_t *container, esdm_dataset_iterator_t **iter) {
  ESDM_DEBUG(__func__);

  return ESDM_SUCCESS;
}

char const * esdm_dataset_name(esdm_dataset_t *d){
  return d->name;
}

int64_t const * esdm_dataset_get_actual_size(esdm_dataset_t *dset){
  if(dset->actual_size){
    return dset->actual_size;
  }
  return dset->dataspace->size;
}

int64_t const * esdm_dataset_get_size(esdm_dataset_t *dset){
  return dset->dataspace->size;
}

esdm_status esdm_dataset_update_size(esdm_dataset_t *d, uint64_t * sizes){
  ESDM_DEBUG(__func__);
  void * tgt;
  if(d->actual_size){
    tgt = d->actual_size;
  }else{
    tgt = d->dataspace->size;
  }
  memcpy(sizes, tgt, sizeof(uint64_t) * d->dataspace->dims);
  return ESDM_SUCCESS;
}


esdm_type_t esdm_dataset_get_type(esdm_dataset_t * d){
  eassert(d);
  return d->dataspace->type;
}

esdm_type_t esdm_dataspace_get_type(esdm_dataspace_t * d){
  eassert(d);
  return d->type;
}
