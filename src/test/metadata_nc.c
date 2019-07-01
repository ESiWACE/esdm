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

/*

netcdf example_1 {  // example of CDL notation for a netCDF dataset

     dimensions:         // dimension names and lengths are declared first
             lat = 5, lon = 10, level = 4, time = unlimited;

     variables:          // variable types, names, shapes, attributes
             float   temp(time,level,lat,lon);
                         temp:long_name     = "temperature";
                         temp:units         = "celsius";
             float   rh(time,lat,lon);
                         rh:long_name = "relative humidity";
                         rh:valid_range = 0.0, 1.0;      // min and max
             int     lat(lat), lon(lon), level(level);
                         lat:units       = "degrees_north";
                         lon:units       = "degrees_east";
                         level:units     = "millibars";
             short   time(time);
                         time:units      = "hours since 1996-1-1";
             // global attributes
                         :source = "Fictional Model Output";

     data:                // optional data assignments
             level   = 1000, 850, 700, 500;
             lat     = 20, 30, 40, 50, 60;
             lon     = -160,-140,-118,-96,-84,-52,-45,-35,-25,-15;
             time    = 12;
             rh      =.5,.2,.4,.2,.3,.2,.4,.5,.6,.7,
                      .1,.3,.1,.1,.1,.1,.5,.7,.8,.8,
                      .1,.2,.2,.2,.2,.5,.7,.8,.9,.9,
                      .1,.2,.3,.3,.3,.3,.7,.8,.9,.9,
                       0,.1,.2,.4,.4,.4,.4,.7,.9,.9;
     }

*/

#include <stdio.h>
#include <stdlib.h>

#include <esdm-datatypes-internal.h>
#include <esdm.h>


// typedef struct {
//   float longitude;
//   char longitude_units[20];
//   char longitude_name[20];
// } longitude_str;
//
// typedef struct {
//   float latitude;
//   char latitude_units[20];
//   char latitude_name[20];
// } latitude_str;
//
// typedef struct {
//   char time_units[100];
//   char time_name[20];
//   char time_calendar[20];
// } time_str;
//
// typedef struct {
//   float scale_factor;
//   float add_offset;
//   long int _FillValue;    // maybe char because of the s
//   long int missing_value; // maybe char because of the s
//   char t2m_units[5];
//   char t2m_name[20];
// } t2m;
//
// typedef struct {
//   float scale_factor;
//   float add_offset;
//   long int _FillValue;    // maybe char because of the s
//   long int missing_value; // maybe char because of the s
//   char t2m_units[5];
//   char t2m_name[20];
// } sund;
//
// typedef struct nc_dims {
//   int longitude;
//   int latitude;
// } nc_dims;
//
// typedef struct {
//   char conventions[10];
//   char history[1000];
// } nc_attr;

void read_test();
static void write_test();

int main() {
  esdm_status ret;

  // printf("\n\nStarting!\n\n");
  ret = esdm_init();
  assert(ret == ESDM_SUCCESS);

  write_test();
  read_test();

  ret = esdm_finalize();
  assert(ret == ESDM_SUCCESS);

  printf("OK\n");

  return 0;
}

static void write_test() {
  esdm_status ret;

  // char *result = NULL;
  // Interaction with ESDM
  esdm_dataspace_t *dataspace = NULL;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  // define dataspace
  int64_t bounds[] = {10, 20};
  int64_t dims = 2; // This value cannot be fixed for the purpose it's being used

  // write the actual metadata

  ret = esdm_container_create("mycontainer", &container);
  ret = esdm_dataspace_create(dims, bounds, SMD_DTYPE_UINT64, &dataspace);

  // NetCDF consists of three types of things

  // 1) Dimensions

  // Dimensions are implicitly part of ESDM when defining the bounds of a dataspace, but they are unnamed
  // So we have to name them

  // 2) Variables

  ret = esdm_dataset_create(container, "myVariable", dataspace, &dataset);
  assert(ret == ESDM_SUCCESS);

  // 3) Attributes

  smd_attr_t *var1 = smd_attr_new("lat", SMD_DTYPE_EMPTY, NULL, 0);
  ret = esdm_dataset_link_attribute(dataset, var1);
  assert(ret == ESDM_SUCCESS);

  int value1 = 5;
  smd_attr_t *attr1 = smd_attr_new("value", SMD_DTYPE_UINT64, &value1, 1);
  ret = smd_attr_link(var1, attr1, 0);
  assert(ret == ESDM_SUCCESS);

  smd_attr_t *attr2 = smd_attr_new("units", SMD_DTYPE_STRING, "degrees_north", 2);
  ret = smd_attr_link(var1, attr2, 0);
  assert(ret == ESDM_SUCCESS);

  smd_attr_t *var2 = smd_attr_new("temp", SMD_DTYPE_EMPTY, NULL, 3);
  ret = esdm_dataset_link_attribute(dataset, var2);
  assert(ret == ESDM_SUCCESS);

  float value2 = 1.2;
  smd_attr_t *attr3 = smd_attr_new("value", SMD_DTYPE_FLOAT, &value2, 4);
  ret = smd_attr_link(var2, attr3, 0);
  assert(ret == ESDM_SUCCESS);

  smd_attr_t *attr4 = smd_attr_new("units", SMD_DTYPE_STRING, "hours since 1996-1-1", 5);
  ret = smd_attr_link(var2, attr4, 0);
  assert(ret == ESDM_SUCCESS);

  // this step shall write out the metadata and make it persistent
  ret = esdm_dataset_commit(dataset);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_container_commit(container);
  assert(ret == ESDM_SUCCESS);

  // remove everything from memory
  ret = esdm_dataset_destroy(dataset);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_container_destroy(container);
  assert(ret == ESDM_SUCCESS);
}

void read_test() {
  esdm_status ret;

  // Interaction with ESDM
  // esdm_dataspace_t *dataspace = NULL;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  ret = esdm_container_retrieve("mycontainer", &container);

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

  // for NetCDF: dims and type
  esdm_dataspace_t *dspace;
  ret = esdm_dataset_get_dataspace(dataset, &dspace);
  assert(ret == ESDM_SUCCESS);
  assert(dspace != NULL);
  esdm_dataspace_print(dspace);
  assert(dspace->type != NULL);
  assert(dspace->dims == 2);

  char type[100];
  char type_e[100];
  smd_type_ser(type, dspace->type);
  smd_type_ser(type_e, SMD_DTYPE_UINT64);
  assert(strcmp(type, type_e) == 0);

  // names of the dims
  // char const *const *names = NULL;
  // ret = esdm_dataset_get_name_dims(dataset, &names);
  // assert(names != NULL);
  // assert(strcmp(names[0], "longitude") == 0);
  // assert(strcmp(names[1], "latitude") == 0);

  // get the attributes
  smd_attr_t *md = NULL;
  ret = esdm_dataset_get_attributes(dataset, &md);
  assert(ret == ESDM_SUCCESS);

  char *txt;
  smd_attr_t *attr, *var1, *var2;
  int *aux1; float *aux2;

  attr = smd_attr_get_child_by_name(md, "lat");

  var1 = smd_attr_get_child_by_name(attr, "value");
  assert(var1 != NULL);
  assert(smd_attr_get_type(var1) == SMD_TYPE_UINT64);

  aux1 = (int *) smd_attr_get_value(var1);
  // assert(aux1 == 5);

  var2 = smd_attr_get_child_by_name(attr, "units");
  assert(var2 != NULL);
  assert(smd_attr_get_type(var2) == SMD_TYPE_STRING);

  txt = (char *)smd_attr_get_value(var2);

  printf("\n\nVariable: Latitude\n");
  printf("value = %d\n", aux1);
  printf("units = %s\n\n", txt);

  attr = smd_attr_get_child_by_name(md, "temp");

  var1 = smd_attr_get_child_by_name(attr, "value");
  assert(var1 != NULL);
  assert(smd_attr_get_type(var1) == SMD_TYPE_FLOAT);

  aux2 = (float *) smd_attr_get_value(var1);
  // assert(aux1 == 5);

  var2 = smd_attr_get_child_by_name(attr, "units");
  assert(var2 != NULL);
  assert(smd_attr_get_type(var2) == SMD_TYPE_STRING);

  txt = (char *)smd_attr_get_value(var2);

  printf("\n\nVariable: Time\n");
  printf("value = %f\n", (double *) aux2);
  printf("units = %s\n\n", txt);

  // char *txt;
  // smd_attr_t *a1;
  // a1 = smd_attr_get_child_by_name(md, "history");
  // assert(a1 != NULL);
  // assert(smd_attr_get_type(a1) == SMD_TYPE_STRING);
  // txt = (char *)smd_attr_get_value(a1);
  // assert(txt != NULL);
  //
  // a1 = smd_attr_get_child_by_name(md, "unit");
  // assert(a1 != NULL);
  // assert(smd_attr_get_type(a1) == SMD_TYPE_STRING);
  // txt = (char *)smd_attr_get_value(a1);
  // assert(strcmp(txt, "Celsius") == 0);

  ret = esdm_dataset_destroy(dataset);
  assert(ret == ESDM_SUCCESS);

  ret = esdm_container_destroy(container);
  assert(ret == ESDM_SUCCESS);
}
