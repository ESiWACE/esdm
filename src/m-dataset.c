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
#include <assert.h>
#include "include/memvol.h"
#include "include/debug.h"
#include "include/memvol-internal.h"


static void* memvol_dataset_create(void* obj, H5VL_loc_params_t loc_params, const char* name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void** req)

{
	puts("------------ memvol_dataset_create() called -------------\n");

//Memory allocation for creating object 
	memvol_object_t* dset_object = (memvol_object_t*)malloc(sizeof(memvol_object_t));

        if( dset_object != NULL) {
            dset_object->type = (memvol_object_type)malloc(sizeof(memvol_object_type));

//Memory allocation for creating dataset structure
            dset_object->subclass = (memvol_dataset_t *)malloc(sizeof(memvol_dataset_t));
      
        }
        else {
	    printf("Can't allocate memory");
	    return 0;
	}
//-------------------------------------------------------------------------------------------
        memvol_group_t* parent_group;
        memvol_object_t* object = (memvol_object_t *)obj;

        if (object->type == GROUP_T) {
             
          parent_group = (memvol_group_t *)object->subclass;

            DEBUG_MESSAGE("parent_group %zu\n", parent_group);
            DEBUG_MESSAGE("parent_group name %d\n", parent_group->name);

        } 
        else if(object->type == DATASET_T) { 
          
           return 0; 
        }
    
        dset_object->type = DATASET_T;

        DEBUG_MESSAGE("dset_object->type %zu\n", dset_object->type);

        memvol_dataset_t*  dataset = dset_object->subclass;

        dataset->name = (char*)malloc(strlen(name) + 1);
	strcpy(dataset->name, name);   // name of the dataset

       // retrieving of dataset, dataspace and link creation property lists
	H5Pget(dcpl_id, H5VL_PROP_DSET_TYPE_ID, &dataset->datatype);    // datatype of the dataset
 
	H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, &dataset->dataspace);  // dataspace

        hssize_t space_number = H5Sget_simple_extent_npoints(dataset->dataspace); 
        DEBUG_MESSAGE("1. space_number = %d\n", space_number);

        H5Pget(dcpl_id, H5VL_PROP_DSET_LCPL_ID, &dataset->lcpl);        // link creation property list

        
	H5Pclose(dcpl_id);

        dataset->loc_group = object; // location group
        dataset->data = NULL;       // raw data 

        DEBUG_MESSAGE("datatype_name %s\n", dataset->name);
	DEBUG_MESSAGE("datatype_value %zu\n", dataset->datatype);
	DEBUG_MESSAGE("dataspace_value %zu\n", dataset->dataspace);
        DEBUG_MESSAGE("link_creation_property_list %zu\n", dataset->lcpl);
        DEBUG_MESSAGE("dataset data %zu \n", dataset->data);
		
        DEBUG_MESSAGE("dataset %zu\n", dataset);

        if (NULL == g_hash_table_lookup(parent_group->children, name)) {
          g_hash_table_insert(parent_group->children, strdup(name), dset_object);  // insertion in the table 
        }
        else { 
           printf("Dataset with the name %s already exists in this group\n", name);
           return 0;
        }
        DEBUG_MESSAGE("dataset_object %zu\n", dset_object);

	return (void *)dset_object;
}

static void* memvol_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                  hid_t dapl_id, hid_t dxpl_id, void **req){

puts("------------ memvol_dataset_open() called -------------\n");

 
  
   memvol_object_t* loc_object = (memvol_object_t *)obj;

   memvol_group_t* parent = (memvol_group_t *)loc_object->subclass;

// opening
    memvol_object_t* dset_object = g_hash_table_lookup(parent->children, name);

     
    //debug Ausgaben
    if (dset_object == NULL) {
        puts("Dataset nicht im angegebenen Parent gefunden!\n");

    } else {

      memvol_dataset_t* dset = (memvol_dataset_t *)dset_object->subclass;

      strcpy(dset->name, name);
//bebug ausgabe
 
      DEBUG_MESSAGE("Opened dataset object %zu \n", dset_object);
      DEBUG_MESSAGE("dataset %zu \n", dset_object->subclass);
      DEBUG_MESSAGE("dataset name %s \n", dset->name);
      DEBUG_MESSAGE("dataset datatype %zu \n", dset->datatype);
      DEBUG_MESSAGE("dataset_dataspace %zu\n", dset->dataspace);

      DEBUG_MESSAGE("dataset data %zu \n", dset->data);
    }
return (void *) dset_object;
}

static herr_t memvol_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
                   hid_t xfer_plist_id, void * buf, void **req){

puts("------------ memvol_dataset_read() called -------------\n");

 htri_t ret;
 hssize_t read_number, n_points;
 size_t size; 

 memvol_object_t* object = (memvol_object_t*)dset;
 memvol_dataset_t*  dataset = (memvol_dataset_t* )object->subclass;

 if (dataset->data == NULL) {
  
    printf("Dataset is empty\n");
    return 0;
  
  }
/*assert, that datatype classes are equal*/  
   assert(H5Tget_class(mem_type_id) == H5Tget_class(dataset->datatype));

/*size of datatype in dataset*/
 size = H5Tget_size(dataset->datatype);
  
/*number of points in dataset*/
 n_points = H5Sget_simple_extent_npoints(dataset->dataspace);

if(mem_space_id == H5S_ALL) {
     read_number = n_points;   //spaces must match
}
else if(mem_space_id > 0){
 /* number of elements to write (if not H5S_ALL)*/
    read_number = H5Sget_simple_extent_npoints(mem_space_id);
}
else {
    DEBUG_MESSAGE("Unappropriate mem_space_id \n");
}

 /*native datatype of dataset datatype*/ 
 hid_t nativ_type = H5Tget_native_type(dataset->datatype, H5T_DIR_ASCEND);

assert(read_number <= n_points);

  if(file_space_id == H5S_ALL){

      if(mem_space_id == H5S_ALL){
            if (mem_type_id == H5T_NATIVE_INT || mem_type_id == H5T_NATIVE_FLOAT) {

		 for(int i = 0; i < read_number; i++){
                     /*read data*/
	             memcpy(buf, dataset->data, n_points * size);
		 }
	    }
      }
      else { /*valid mem_space_id*/
      // to do
      } 
   }
   else { /*valid file_space_id*/
   
      if(mem_space_id == H5S_ALL){

      // to do
      }
      else {/*valid mem_space_id*/
       // to do
      } 
   }
return 1;
} 

static herr_t memvol_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
                    hid_t xfer_plist_id, const void * buf, void **req) {

puts("------------ memvol_dataset_write() called -------------\n");

 int dims;
 htri_t ret;
 hsize_t *dim;
 hsize_t *maxdim;
 hssize_t write_number = 0;
  hssize_t  n_points;
 herr_t status;
 size_t type_size, size; 
 H5T_class_t class;
 ssize_t length;
 char* type_name;
 static H5S_sel_type file_sel_type, mem_sel_type;

 memvol_object_t* object = (memvol_object_t*) dset;
 memvol_dataset_t*  dataset = (memvol_dataset_t* )object->subclass;

DEBUG_MESSAGE("datatype %zu\n", dataset->datatype);

/* class of datatype */
 class = H5Tget_class(dataset->datatype); 


/*size of datatype in dataset*/
 size = H5Tget_size(dataset->datatype);

  
/*number of points in dataset*/
 n_points = H5Sget_simple_extent_npoints(dataset->dataspace);


//assert, that datatype classes are equal 
assert(H5Tget_class(mem_type_id) == H5Tget_class(dataset->datatype));

/*size of the mem_type in Bytes*/
type_size = H5Tget_size(mem_type_id); 

if(mem_space_id == H5S_ALL) { 
   write_number = n_points;
  mem_sel_type = H5S_SEL_ALL;

DEBUG_MESSAGE("write_number %zu\n", write_number);
  
}
else {
 /* number of elements to write (if not H5S_ALL)*/
    write_number = H5Sget_simple_extent_npoints(mem_space_id);
}
if(file_space_id == H5S_ALL) { 
 
  file_sel_type = H5S_SEL_ALL;
  
}
else {
 
/*type of file selection*/
   file_sel_type = H5Sget_select_type(file_space_id);
}

if(dataset->data == NULL) {
   dataset->data =  malloc(n_points * size); 

DEBUG_MESSAGE("data %zu\n", dataset->data);
}


/*rank of the dataset dataspace*/
dims = H5Sget_simple_extent_ndims(dataset->dataspace); 
DEBUG_MESSAGE("dims %d\n", dims);

/* dimention size and maximal size*/
//status = H5Sget_simple_extent_dims(dataset->dataspace, dim, maxdim);


/*assert, that data passt in dataset container*/
//assert(write_number <= n_points); 


if(file_space_id == H5S_ALL){
      if(mem_space_id == H5S_ALL){
	   /*complete write*/
 
	   if (mem_type_id == H5T_NATIVE_INT || mem_type_id == H5T_NATIVE_FLOAT) {
                                    
		   for(int i = 0; i < write_number; i++){
                   	  memcpy(dataset->data, buf, n_points * size);
                       
                   }
                   write_number = n_points;
            }
	   else {
		   DEBUG_MESSAGE("Unhandled type\n");
		   exit(1);
	   }
   }
   else if (mem_space_id > 0){ /*valid mem_space_id*/

       switch(mem_sel_type) {

	  case H5S_SEL_NONE: /* no selection */ break;
	  case H5S_SEL_POINTS: 
          /* to do */   break;
	
	  case H5S_SEL_HYPERSLABS: 
          /*to do*/   break; 
	  
	  case H5S_SEL_ALL: 
            for(int i = 0; i < write_number; i++){
               dataset->data =  &buf;  /*to do !!*/
            }
          break;
	  
	  default: 
             DEBUG_MESSAGE("unknown selection type\n");
	     ret = 0;
          break;
	} 
        
    } 
}
else { /*valid file_space_id*/

    if(mem_space_id == H5S_ALL){

       switch(file_sel_type){

	case H5S_SEL_NONE:   /*no points selected*/  break; 
        case H5S_SEL_POINTS:   /* to do*/
          break;
	
	case H5S_SEL_HYPERSLABS: /* to do */
         break;
	
	case H5S_SEL_ALL: 
          for(int i = 0; i <= write_number; i++){
               dataset->data =  &buf;
          }
          break;
	default: 
           DEBUG_MESSAGE("unknown selection type\n");
	   ret = 0;
	   break;   
       }
    }
    else {/*valid mem_space_id*/
 /*combination of POINTS-POINTS, POINTS-HYPERSLAB, POINTS-SELL_ALL, HYP-POINTS, HYP-HYP, HYP-SEL_ALL,
  SEL_ALL-POINTS, SEL_ALL-HYPER, SEL_ALL-SEL_ALL; NONE mit anderen*/

/*to do*/
    } 
}
memvol_object_t* parent = dataset->loc_group;
memvol_group_t* parent_group = (memvol_group_t*)parent->subclass; 
g_hash_table_insert(parent_group->children, strdup(dataset->name), object);  // insertion of the actual dataset in the table 
 return 1;
}

static  herr_t memvol_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){

puts("------------ memvol_dataset_get() called -------------\n");

   memvol_object_t* object = (memvol_object_t*)dset;
   memvol_dataset_t*  dataset = (memvol_dataset_t* )object->subclass;

   herr_t ret_value = 1;
   
    switch(get_type) {

           case H5VL_DATASET_GET_DAPL: 
           {	
		hid_t *ret_id = va_arg (arguments, hid_t *);
                printf("Access property list %p\n", *ret_id);
                              
		break;
           }
           case H5VL_DATASET_GET_DCPL:
           {
		hid_t *ret_id = va_arg (arguments, hid_t *);
		printf("Creation property list %p\n", *ret_id);
                        
		break;
           }
           case H5VL_DATASET_GET_OFFSET:
           {
	         haddr_t *ret = va_arg (arguments, haddr_t *);
		printf("The offset of the dataset %d \n", *ret);
		 
			   /* Set return value */
			   //*ret = H5D__get_offset(dset);
			   //if(!H5F_addr_defined(*ret))
			//	   *ret = HADDR_UNDEF;
		break;
           }
           case H5VL_DATASET_GET_SPACE:
           {
		hid_t *ret_id = va_arg (arguments, hid_t *);
		printf("Dataspace %p\n", *ret_id);
                
  		break;

           }
	   case H5VL_DATASET_GET_SPACE_STATUS:
	   {
		H5D_space_status_t *allocation = va_arg (arguments, H5D_space_status_t *);
		printf("Space status %p\n", *allocation);
                
		break;
           }
           case H5VL_DATASET_GET_STORAGE_SIZE:
           {
		hsize_t *ret = va_arg (arguments, hsize_t *);
		printf("Storage size %p\n", *ret);
                
		break;
           }
           case H5VL_DATASET_GET_TYPE:
           {
      		hid_t *ret_id = va_arg (arguments, hid_t *);
		printf("Datatype %d\n", *ret_id);
                
		break;
           }
	   default:{
	   	DEBUG_MESSAGE("unknown type found\n");
		ret_value = 0;
	}
   }
   return ret_value;       
}
 
static herr_t memvol_dataset_close(void* dset, hid_t dxpl_id, void** req) {

    puts("------------ memvol_dataset_close() called -------------\n");
  
    memvol_object_t *object = (memvol_object_t*) dset;
        
    memvol_dataset_t *dataset = object->subclass;
/*
    allocated memory will be free by closing of the location group
       
**/
    return 1;


}
