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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("DATATYPES", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("DATATYPES", fmt, __VA_ARGS__)

extern esdm_instance_t esdm;

// Container //////////////////////////////////////////////////////////////////

esdm_status esdm_container_create(const char *name, esdm_container_t **out_container) {
  ESDM_DEBUG(__func__);
  esdm_container_t *container = (esdm_container_t *)malloc(sizeof(esdm_container_t));

  container->name = strdup(name);
  container->status = ESDM_STATUS_DIRTY;

  *out_container = container;

  return ESDM_SUCCESS;
}

esdm_status esdm_container_retrieve(const char *name, esdm_container_t **out_container) {
  ESDM_DEBUG(__func__);
  esdm_container_t *container = (esdm_container_t *)malloc(sizeof(esdm_container_t));

  // TODO: retrieve from MD
  // TODO: retrieve associated data

  container->name = strdup(name);
  container->status = ESDM_STATUS_DIRTY;

  *out_container = container;

  return ESDM_SUCCESS;
}

esdm_status esdm_container_commit(esdm_container_t *container) {
  ESDM_DEBUG(__func__);
  // md callback create/update container
  esdm_status status = esdm.modules->metadata_backend->callbacks.container_create(esdm.modules->metadata_backend, container);

  // Also commit uncommited datasets of this container?

  return status;
}

esdm_status esdm_container_destroy(esdm_container_t *container) {
  ESDM_DEBUG(__func__);
  free(container);

  return ESDM_SUCCESS;
}

// Fragment ///////////////////////////////////////////////////////////////////

/**
 *	TODO: there should be a mode to auto-commit on creation?
 *
 *	How does this integrate with the scheduler? On auto-commit this merely beeing pushed to sched for dispatch?
 */

esdm_status esdm_fragment_create(esdm_dataset_t *dataset, esdm_dataspace_t *subspace, void *buf, esdm_fragment_t **out_fragment) {
  ESDM_DEBUG(__func__);
  esdm_fragment_t *fragment = (esdm_fragment_t *)malloc(sizeof(esdm_fragment_t));

  int64_t i;
  for (i = 0; i < subspace->dims; i++) {
    DEBUG("dim %d, size=%d (%p)\n", i, subspace->size[i], subspace->size);
    assert(subspace->size[i] > 0);
  }

  uint64_t elements = esdm_dataspace_element_count(subspace);
  int64_t bytes = elements * esdm_sizeof(subspace->type);
  DEBUG("Entries in subspace: %d x %d bytes = %d bytes \n", elements, esdm_sizeof(subspace->type), bytes);

  fragment->metadata = malloc(ESDM_MAX_SIZE);
  fragment->metadata->json = (char *)(fragment->metadata) + sizeof(esdm_metadata_t);
  fragment->metadata->json[0] = 0;
  fragment->metadata->size = 0;

  fragment->dataset = dataset;
  fragment->dataspace = subspace;
  fragment->buf = buf; // zero copy?
  fragment->elements = elements;
  fragment->bytes = bytes;
  fragment->status = ESDM_STATUS_DIRTY;

  *out_fragment = fragment;

  return ESDM_SUCCESS;
}

esdm_status esdm_dataspace_overlap_str(esdm_dataspace_t *a, char delim_c, char *str_offset, char *str_size, esdm_dataspace_t **out_space) {
  //printf("str: %s %s\n", str_size, str_offset);

  const char delim[] = {delim_c, 0};
  char *save = NULL;
  char *cur = strtok_r(str_offset, delim, &save);
  if (cur == NULL) return ESDM_ERROR;

  int64_t off[a->dims];
  int64_t size[a->dims];

  for (int d = 0; d < a->dims; d++) {
    if (cur == NULL) return ESDM_ERROR;
    off[d] = atol(cur);
    if (off[d] < 0) return ESDM_ERROR;
    //printf("o%ld,", off[d]);
    save[-1] = delim_c; // reset the overwritten text
    cur = strtok_r(NULL, delim, &save);
  }
  //printf("\n");
  if (str_size != NULL) {
    cur = strtok_r(str_size, delim, &save);
  }

  for (int d = 0; d < a->dims; d++) {
    if (cur == NULL) return ESDM_ERROR;
    size[d] = atol(cur);
    if (size[d] < 0) return ESDM_ERROR;
    //printf("s%ld,", size[d]);
    if (save[0] != 0) save[-1] = delim_c;
    cur = strtok_r(NULL, delim, &save);
  }
  //printf("\n");
  if (cur != NULL) {
    return ESDM_ERROR;
  }

  // dims match, now check for overlap
  for (int d = 0; d < a->dims; d++) {
    int o1 = a->offset[d];
    int s1 = a->size[d];

    int o2 = off[d];
    int s2 = size[d];

    //printf("%ld %ld != %ld %ld\n", o1, s1, o2, s2);
    if (o1 + s1 <= o2) return ESDM_ERROR;
    if (o2 + s2 <= o1) return ESDM_ERROR;
  }
  if (out_space != NULL) {
    // TODO always go to the parent space
    esdm_dataspace_t *parent = a->subspace_of == NULL ? a : a->subspace_of;
    esdm_dataspace_subspace(parent, a->dims, size, off, out_space);
  }
  return ESDM_SUCCESS;
}

esdm_status esdm_fragment_retrieve(esdm_fragment_t *fragment) {
  ESDM_DEBUG(__func__);

  json_t *root = load_json(fragment->metadata->json);
  json_t *elem;
  elem = json_object_get(root, "data");

  // Call backend
  esdm_backend_t *backend = fragment->backend; // TODO: decision component, upon many
  backend->callbacks.fragment_retrieve(backend, fragment, elem);

  return ESDM_SUCCESS;
}

void esdm_dataspace_string_descriptor(char *string, esdm_dataspace_t *dataspace) {
  ESDM_DEBUG(__func__);
  int pos = 0;

  int64_t dims = dataspace->dims;
  int64_t *size = dataspace->size;
  int64_t *offset = dataspace->offset;

  // offset to string
  int64_t i;
  pos += sprintf(&string[pos], "%ld", offset[0]);
  for (i = 1; i < dims; i++) {
    DEBUG("dim %d, offset=%ld (%p)\n", i, offset[i], offset);
    pos += sprintf(&string[pos], ",%ld", offset[i]);
  }

  // size to string
  pos += sprintf(&string[pos], ",%ld", size[0]);
  for (i = 1; i < dims; i++) {
    DEBUG("dim %d, size=%ld (%p)\n", i, size[i], size);
    pos += sprintf(&string[pos], ",%ld", size[i]);
  }
  DEBUG("Descriptor: %s\n", string);
}

esdm_status esdm_fragment_commit(esdm_fragment_t *f) {
  ESDM_DEBUG(__func__);
  esdm_metadata_t *m = f->metadata;
  esdm_dataspace_t *d = f->dataspace;

  m->size += sprintf(&m->json[m->size], "{\"plugin\" : \"%s\", \"id\" : \"%s\", \"size\": \"", f->backend->name, f->backend->config->id);

  m->size += sprintf(&m->json[m->size], "%ld", d->size[0]);
  for (int i = 1; i < d->dims; i++) {
    m->size += sprintf(&m->json[m->size], "x%ld", d->size[i]);
  }

  m->size += sprintf(&m->json[m->size], "\", \"offset\" :\"");
  m->size += sprintf(&m->json[m->size], "%ld", d->offset[0]);
  for (int i = 1; i < d->dims; i++) {
    m->size += sprintf(&m->json[m->size], "x%ld", d->offset[i]);
  }

  m->size += sprintf(&m->json[m->size], "\", \"data\" :");

  f->backend->callbacks.fragment_update(f->backend, f);
  m->size += sprintf(&m->json[m->size], "}");

  // Announce to metadata coordinator
  esdm.modules->metadata_backend->callbacks.fragment_update(esdm.modules->metadata_backend, f);

  f->status = ESDM_STATUS_PERSISTENT;

  return ESDM_SUCCESS;
}

esdm_status esdm_fragment_destroy(esdm_fragment_t *fragment) {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}

esdm_status esdm_fragment_serialize(esdm_fragment_t *fragment, void **out) {
  ESDM_DEBUG(__func__);

  return ESDM_SUCCESS;
}

esdm_status esdm_fragment_deserialize(void *serialized_fragment, esdm_fragment_t **_out_fragment) {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}

// Dataset ////////////////////////////////////////////////////////////////////


void esdm_dataset_init(esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset){
  esdm_dataset_t *d = (esdm_dataset_t *)malloc(sizeof(esdm_dataset_t));

  d->dims_dset_id = NULL;
  d->name = strdup(name);
  d->container = container;
  d->dataspace = dataspace;
	esdm_metadata_t_init_(& d->metadata);
  *out_dataset = d;
}


esdm_status esdm_dataset_create(esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset) {
  ESDM_DEBUG(__func__);
	esdm_dataset_t *d;
	esdm_dataset_init(container, name, dataspace, &d);
	*out_dataset = d;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_retrieve_md_load(esdm_dataset_t *dset, char ** out_md, int * out_size){
	return esdm.modules->metadata_backend->callbacks.dataset_retrieve(esdm.modules->metadata_backend, dset, out_md, out_size);
}

esdm_status esdm_dataset_retrieve_md_parse(esdm_dataset_t *d, char * md, int size){
	esdm_status ret;
	char * js = md;
	d->metadata = (esdm_metadata_t *)malloc(sizeof(esdm_metadata_t));

  // first strip the attributes
  size_t parsed = smd_attr_create_from_json(js + 1, size, & d->metadata->attr);
  js += 1 + parsed;
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
  elem = json_object_get(root, "dims");
  int dims = json_integer_value(elem);
  elem = json_object_get(root, "size");
  size_t arrsize = json_array_size(elem);
  if (dims != arrsize) {
		free(d);
		free(js);
    return ESDM_ERROR;
  }
  int64_t sizes[dims];
  for (int i = 0; i < dims; i++) {
    sizes[i] = json_integer_value(json_array_get(elem, i));
  }
  ret = esdm_dataspace_create(dims, sizes, type, &d->dataspace);
  if (ret != ESDM_SUCCESS) {
    return ret;
  }
  elem = json_object_get(root, "dims_dset_id");
  arrsize = json_array_size(elem);
  if (dims != arrsize) {
    return ESDM_ERROR;
  }
  char *strs[dims];
  for (int i = 0; i < dims; i++) {
    strs[i] = (char *)json_string_value(json_array_get(elem, i));
  }
  esdm_dataset_name_dims(d, strs);
	return ESDM_SUCCESS;
}

esdm_status esdm_dataset_retrieve(esdm_container_t *container, const char *name, esdm_dataset_t **out_dataset) {
  ESDM_DEBUG(__func__);
	char * buff;
  int size;
  esdm_dataset_t *d;
	esdm_dataset_init(container, name, NULL, & d);

  *out_dataset = NULL;

  esdm_status ret = esdm_dataset_retrieve_md_load(d, & buff, & size);
	if(ret != ESDM_SUCCESS){
		free(d);
		return ret;
	}
	ret = esdm_dataset_retrieve_md_parse(d, buff, size);
	free(buff);
	if(ret != ESDM_SUCCESS){
		free(d);
		return ret;
	}
  *out_dataset = d;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_commit(esdm_dataset_t *d) {
  ESDM_DEBUG(__func__);
  // TODO

	const int len = 100000;
  char buff[len];
	char * js = buff;
  const char *jso = js;
  js += sprintf(js, "{");
  js += smd_attr_ser_json(js, d->metadata->attr) - 1;
  js += snprintf(js, len + jso - js, ",\"typ\":\"");
  js += smd_type_ser(js, d->dataspace->type) - 1;
  js += snprintf(js, len + jso - js, "\",\"dims\":%" PRId64 ",\"size\":[%" PRId64, d->dataspace->dims, d->dataspace->size[0]);
  for (int i = 1; i < d->dataspace->dims; i++) {
    js += snprintf(js, len + jso - js, ",%" PRId64, d->dataspace->size[i]);
  }
  js += snprintf(js, len + jso - js, "]");
  if (d->dims_dset_id != NULL) {
    js += snprintf(js, len + jso - js, ",\"dims_dset_id\":[\"%s\"", d->dims_dset_id[0]);
    for (int i = 1; i < d->dataspace->dims; i++) {
      js += snprintf(js, len + jso - js, ",\"%s\"", d->dims_dset_id[i]);
    }
    js += snprintf(js, len + jso - js, "]");
  }
  js += snprintf(js, len + jso - js, "}");

  int md_size = (js - jso);

  // md callback create/update container
  esdm_status ret = esdm.modules->metadata_backend->callbacks.dataset_commit(esdm.modules->metadata_backend, d, buff, md_size);
  return ret;
}

esdm_status esdm_dataset_update(esdm_dataset_t *dataset) {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_destroy(esdm_dataset_t *dataset) {
  ESDM_DEBUG(__func__);
  //	free(dataset->name);
  free(dataset->metadata);
  dataset->metadata = NULL;

  if (dataset->dims_dset_id == NULL) {
    free(dataset->dims_dset_id);
  }
  	free(dataset->container);
  	free(dataset->dataspace);
  free(dataset);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_get_attributes(esdm_dataset_t *dataset, smd_attr_t **out_metadata) {
  assert(dataset->metadata->attr != NULL);
  *out_metadata = dataset->metadata->attr;
  return ESDM_SUCCESS;
}

// Dataspace //////////////////////////////////////////////////////////////////

esdm_status esdm_dataspace_create(int64_t dims, int64_t *sizes, esdm_type_t type, esdm_dataspace_t **out_dataspace) {
  ESDM_DEBUG(__func__);
  assert(dims >= 0);
  assert(!dims || sizes);
  assert(out_dataspace);

  esdm_dataspace_t *dataspace = (esdm_dataspace_t *)malloc(sizeof(esdm_dataspace_t));

  dataspace->dims = dims;
  dataspace->size = (int64_t *)malloc(sizeof(int64_t) * dims);
  dataspace->type = type;
  dataspace->offset = (int64_t *)malloc(sizeof(int64_t) * dims);
  dataspace->subspace_of = NULL;

  memcpy(dataspace->size, sizes, sizeof(int64_t) * dims);
  memset(dataspace->offset, 0, sizeof(int64_t) * dims);

  DEBUG("New dataspace: dims=%d\n", dataspace->dims);

  *out_dataspace = dataspace;

  return ESDM_SUCCESS;
}

uint8_t esdm_dataspace_overlap(esdm_dataspace_t *a, esdm_dataspace_t *b) {
  // TODO: allow comparison of spaces of different size? Alternative maybe to transform into comparable space, provided a mask or dimension index mapping

  if (a->dims != b->dims) {
    // dims do not match so, we say they can not overlap
    return 0;
  }

  return 0;
}

/**
 * this could also be the fragment???
 *
 *
 */
esdm_status esdm_dataspace_subspace(esdm_dataspace_t *dataspace, int64_t dims, int64_t *size, int64_t *offset, esdm_dataspace_t **out_dataspace) {
  ESDM_DEBUG(__func__);

  esdm_dataspace_t *subspace = NULL;

  if (dims == dataspace->dims) {
    // replicate original space
    subspace = (esdm_dataspace_t *)malloc(sizeof(esdm_dataspace_t));
    memcpy(subspace, dataspace, sizeof(esdm_dataspace_t));

    // populate subspace members
    subspace->size = (int64_t *)malloc(sizeof(int64_t) * dims);
    subspace->offset = (int64_t *)malloc(sizeof(int64_t) * dims);
    subspace->subspace_of = dataspace;

    // make copies where necessary
    memcpy(subspace->size, size, sizeof(int64_t) * dims);
    memcpy(subspace->offset, offset, sizeof(int64_t) * dims);

    for (int64_t i = 0; i < dims; i++) {
      DEBUG("dim %d, size=%ld off=%ld", i, size[i], offset[i]);
      assert(size[i] > 0);
    }
  } else {
    ESDM_ERROR("Subspace dims do not match original space.");
  }

  *out_dataspace = subspace;

  return ESDM_SUCCESS;
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
  printf("))");
}

void esdm_fragment_print(esdm_fragment_t *f) {
  printf("FRAGMENT(%p,", (void *)f);
  esdm_dataspace_print(f->dataspace);
  if (f->metadata) {
    printf(", md(\"%s\")", f->metadata->json);
  }
  printf(")");
}

esdm_status esdm_dataspace_destroy(esdm_dataspace_t *dataspace) {
  ESDM_DEBUG(__func__);
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
  assert(subspace->size != NULL);
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

esdm_status esdm_metadata_t_init_(esdm_metadata_t **output_metadata) {
  ESDM_DEBUG(__func__);

  esdm_metadata_t *md = (esdm_metadata_t *)malloc(sizeof(esdm_metadata_t) + 10000);
  assert(md != NULL);
  *output_metadata = md;
  md->buff_size = 10000;
  md->size = 0;
  md->json = ((char *)md) + sizeof(esdm_metadata_t);
  md->attr = smd_attr_new("Variables", SMD_DTYPE_EMPTY, NULL, 0);

  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_name_dims(esdm_dataset_t *d, char **names) {
  ESDM_DEBUG(__func__);
  assert(d != NULL);
  assert(names != NULL);
  int dims = d->dataspace->dims;
  int size = 0;
  // compute size and check that varname is conform
  for (int i = 0; i < dims; i++) {
    size += strlen(names[i]) + 1;
    if (!ea_is_valid_name(names[i])) {
      return ESDM_ERROR;
    }
  }
  if (d->dims_dset_id != NULL) {
    free(d->dims_dset_id);
  }
  d->dims_dset_id = (char **)malloc(dims * sizeof(void *) + size);
  char *posVar = (char *)d->dims_dset_id + dims * sizeof(void *);
  for (int i = 0; i < dims; i++) {
    d->dims_dset_id[i] = posVar;
    strcpy(posVar, names[i]);
    posVar += 1 + strlen(names[i]);
  }
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_get_name_dims(esdm_dataset_t *d, char const *const **out_names) {
  assert(d != NULL);
  assert(out_names != NULL);
  *out_names = (char const *const *)d->dims_dset_id;
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_link_attribute(esdm_dataset_t *dset, smd_attr_t *attr) {
  ESDM_DEBUG(__func__);
  smd_link_ret_t ret = smd_attr_link(dset->metadata->attr, attr, 0);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_iterator(esdm_container_t *container, esdm_dataset_iterator_t **iter) {
  ESDM_DEBUG(__func__);

  return ESDM_SUCCESS;
}
