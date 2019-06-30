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

/*
 * Test to check the metadata APIs of ESDM.
 */

#include <stdio.h>
#include <stdlib.h>

#include <esdm-datatypes-internal.h>
#include <esdm.h>

typedef struct {
  float longitude;
  char longitude_units[20];
  char longitude_name[20];
} longitude_str;

typedef struct {
  float latitude;
  char latitude_units[20];
  char latitude_name[20];
} latitude_str;

typedef struct {
  char time_units[100];
  char time_name[20];
  char time_calendar[20];
} time_str;

typedef struct {
  float scale_factor;
  float add_offset;
  long int _FillValue;    // maybe char because of the s
  long int missing_value; // maybe char because of the s
  char t2m_units[5];
  char t2m_name[20];
} t2m;

typedef struct {
  float scale_factor;
  float add_offset;
  long int _FillValue;    // maybe char because of the s
  long int missing_value; // maybe char because of the s
  char t2m_units[5];
  char t2m_name[20];
} sund;

typedef struct nc_dims {
  int longitude;
  int latitude;
} nc_dims;

typedef struct {
  char conventions[10];
  char history[1000];
} nc_attr;

int convert_smd_to_metadata(smd_attr_t *smd_file, esdm_metadata_t **out);
//esdm_status esdm_dataset_link_metadata (esdm_dataset_t *dataset, smd_attr_t *new);

static void write_test() {
  esdm_status ret;

  char *result = NULL;
  // Interaction with ESDM
  esdm_dataspace_t *dataspace = NULL;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset     = NULL;

  // define dataspace
  int64_t bounds[] = {10, 20};
  // write the actual metadata

  ret = esdm_container_t_create("mycontainer", &container);
  ret = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);

  // NetCDF consists of three types of things
  // 1) Dimensions
  // Dimensions are implicitly part of ESDM when defining the bounds of a dataspace, but they are unnamed
  // So we have to name them

  // 2) Variables
  ret = esdm_dataset_create(container, "myVariable", dataspace, &dataset);
  assert(ret == ESDM_SUCCESS);

  char *names[] = {"longitude", "latitude"};
  ret           = esdm_dataset_name_dimensions(dataset, 2, names);
  assert(ret == ESDM_SUCCESS);

  // 3) Attributes
  char *str         = {"This is some history"};
  smd_attr_t *attr1 = smd_attr_new("history", SMD_DTYPE_STRING, str, 0);
  ret               = esdm_dataset_link_attribute(dataset, attr1);
  assert(ret == ESDM_SUCCESS);

  char *unit        = {"Celsius"};
  smd_attr_t *attr2 = smd_attr_new("unit", SMD_DTYPE_STRING, unit, 1);
  ret               = esdm_dataset_link_attribute(dataset, attr2);
  assert(ret == ESDM_SUCCESS);

  // this step shall write out the metadata and make it persistent
  ret = esdm_dataset_commit(dataset);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_container_t_commit(container);
  assert(ret == ESDM_SUCCESS);

  // remove everything from memory
  ret = esdm_dataset_destroy(dataset);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_container_t_destroy(container);
  assert(ret == ESDM_SUCCESS);
}

void read_test() {
  esdm_status ret;

  // Interaction with ESDM
  esdm_dataspace_t *dataspace = NULL;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset     = NULL;

  ret = esdm_container_t_retrieve("mycontainer", &container);

  // NetCDF consists of three types of things
  // 1) Dimensions
  // Dimensions are implicitly part of ESDM when defining the bounds of a dataspace, but they are unnamed
  // So we have to name them

  //	esdm_dataset_t* esdm_dataset_retrieve(esdm_container_t *container, const char* name)
  //	static int dataset_retrieve(esdm_backend_t* backend, esdm_dataset_t *dataset)

  // TODO later:
  // esdm_dataset_iterator_t * iter;
  // ret = esdm_dataset_iterator(container, & iter);
  // assert(ret == ESDM_SUCCESS);

  ret = esdm_dataset_retrieve(container, "myVariable", &dataset);
  assert(ret == ESDM_SUCCESS);

  // for NetCDF: dimensions
  esdm_dataspace_t *dspace;
  //ret = esdm_dataset_get_dataspace(dataset, & dspace);
  assert(ret == ESDM_SUCCESS);
  //esdm_dataspace_print(dspace);

  // names of the dimensions

  // get datatype

  // get the attributes
  smd_attr_t *md = NULL;
  ret            = esdm_dataset_get_attributes(dataset, &md);
  assert(ret == ESDM_SUCCESS);

  char *txt;
  smd_attr_t *a1;
  a1 = smd_attr_get_child_by_name(md, "history");
  assert(a1 != NULL);
  assert(smd_attr_get_type(a1) == SMD_TYPE_STRING);
  txt = (char *)smd_attr_get_value(a1);
  assert(txt != NULL);

  a1 = smd_attr_get_child_by_name(md, "unit");
  assert(a1 != NULL);
  assert(smd_attr_get_type(a1) == SMD_TYPE_STRING);
  txt = (char *)smd_attr_get_value(a1);
  assert(strcmp(txt, "Celsius") == 0);

  ret = esdm_dataset_destroy(dataset);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_container_t_destroy(container);
  assert(ret == ESDM_SUCCESS);
}

int main() {
  esdm_status ret;

  ret = esdm_init();
  assert(ret == ESDM_SUCCESS);

  write_test();
  read_test();

  ret = esdm_finalize();
  assert(ret == ESDM_SUCCESS);

  printf("OK\n");

  return 0;
}
