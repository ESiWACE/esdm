// This file is part of h5-memvol.
// 
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string.h>


static void* memvol_dataset_create(void* object, H5VL_loc_params_t loc_params, const char* name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void** req)
   
{
    hid_t* datatype_value = NULL;
    hid_t* dataspace_value = NULL;
    herr_t t,t1;


    puts("------------ memvol_dataset_create() called ------------");    
    printf("dcpl_id %p \n ", dcpl_id);
   
   memvol_dataset_t*  dataset = (memvol_dataset_t*)malloc(sizeof(memvol_dataset_t));
printf("dataset %p \n ", dataset);


// name of dataset
     dataset->name = (char*)malloc(strlen(name));

//------------Debug Ausgabe-------------------------
printf("dataset->name %p \n ", dataset->name);
printf("name %s \n ", name);
//--------------------------------------------------


// retrieving of datatype
 printf("datatype_value %p \n ", datatype_value);

    H5Pget(dcpl_id, H5VL_PROP_DSET_TYPE_ID, &datatype_value);

//------------Degub Ausbage-------------------------
printf("datatype_value %p \n ", datatype_value);
printf("datatype_value %p \n ", &datatype_value);
//--------------------------------------------------
   
    dataset->datatype = (hid_t*) malloc(sizeof(datatype_value));
printf("dataset->datatype %p \n ", dataset->datatype);



// retrieving of dataspace
 printf("dataspace_value %p \n ", dataspace_value);

    H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, &dataspace_value);

//------------Debug Ausgabe-------------------------
 printf("dataspace_value %p \n ", dataspace_value);
printf("dataspace_value %p \n ", &dataspace_value);
//--------------------------------------------------

    dataset->dataspace = (void*) malloc(sizeof(dataspace_value));

// retrieving of link creation property list´s
//   H5Pget(dcpl_id, H5VL_PROP_DSET_LCPL_ID, &lcpl);
//   dataset->lcpl = malloc(sizeof(*lcpl));

printf("dataset %p \n ", dataset);
 return (void *)dataset;
}

static void* memvol_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                  hid_t dapl_id, hid_t dxpl_id, void **req){
memvol_dataset_t*  dataset; 
//to do 

return (void*) dataset;
}

static herr_t memvol_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
                   hid_t xfer_plist_id, void * buf, void **req){

//to do
return 1;
}

static herr_t memvol_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
                    hid_t xfer_plist_id, const void * buf, void **req){

//to do
return 1;
}

static  herr_t memvol_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){

  
   hid_t *ret_id;
   haddr_t *ret;
   memvol_dataset_t*  dataset; 
   H5D_space_status_t *allocation;

   dataset = (memvol_dataset_t* )dset;


if(get_type ==  H5VL_DATASET_GET_DAPL){

   //to do
    return 1;
}
                 
 else if(get_type == H5VL_DATASET_GET_DCPL) {

   //to do 
    return 1;
 }                
 else if(get_type == H5VL_DATASET_GET_OFFSET) {

    //to do
    return 1;
 }              
 else if(get_type == H5VL_DATASET_GET_SPACE) {
   //
    return 1;

 }                 
 else if(get_type == H5VL_DATASET_GET_SPACE_STATUS) {

   //
   return 1;

 }          
 else if(get_type == H5VL_DATASET_GET_STORAGE_SIZE) {
//
    return 1;
 }  
 else if(get_type == H5VL_DATASET_GET_TYPE) {
//
    return 1;
 }                   
 else {
    printf("Error");
    return -1;
 }
}
 
static herr_t memvol_dataset_close(void* dset, hid_t dxpl_id, void** req) {

    printf("------------ memvol_dataset_close() called -------------\n");

    memvol_dataset_t *dataset = (memvol_dataset_t*) dset;

    free(dataset->name);
    free(dataset->datatype);
    free(dataset->dataspace);
    free(dataset);

    return 1;
}
