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
//#include <assert.h>
#include "include/memvol.h"
#include "include/debug.h"
#include "include/memvol-internal.h"


static void* memvol_dataset_create(void* obj, H5VL_loc_params_t loc_params, const char* name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void** req)

{
	puts("------------ memvol_dataset_create() called -------------\n");

	memvol_dataset_t*  dataset = (memvol_dataset_t*)calloc(1, sizeof(memvol_dataset_t));
	memvol_object_t* dset_object = (memvol_object_t*)calloc(1, sizeof(memvol_object_t));

        dset_object->type = (memvol_object_type)calloc(1, sizeof(memvol_object_type));
        dset_object->subclass = (memvol_dataset_t *)calloc(1, sizeof(memvol_dataset_t));

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

        dset_object->subclass = dataset;

	dataset->name = (char*)malloc(strlen(name) + 1);
	strcpy(dataset->name, name);

       // retrieving of dataset, dataspace and link creation property lists
	H5Pget(dcpl_id, H5VL_PROP_DSET_TYPE_ID, &dataset->datatype);
	H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, &dataset->dataspace);
        H5Pget(dcpl_id, H5VL_PROP_DSET_LCPL_ID, &dataset->lcpl);
	
        DEBUG_MESSAGE("datatype_name %s\n", dataset->name);
	DEBUG_MESSAGE("datatype_value %zu\n", dataset->datatype);
	DEBUG_MESSAGE("dataspace_value %zu\n", dataset->dataspace);
        DEBUG_MESSAGE("link_creation_property_list %zu\n", dataset->lcpl);

		
        DEBUG_MESSAGE("dataset %zu\n", dataset);

        g_hash_table_insert(parent_group->children, strdup(name), dset_object);
       DEBUG_MESSAGE("dataset_object %zu\n", dset_object);
	return (void *)dset_object;
}

static void* memvol_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                  hid_t dapl_id, hid_t dxpl_id, void **req){

puts("------------ memvol_dataset_open() called -------------\n");


   
    memvol_object_t* loc_object = (memvol_object_t *)obj;

 //   memvol_dataset_t*  dataset = (memvol_dataset_t*)malloc(sizeof(memvol_dataset_t));
  //  memvol_object_type* type =

 //    dataset->name = (char*)malloc(strlen(name) +1);
    strcpy(dataset->name, name);

    memvol_group_t* parent = (memvol_group_t *)loc_object->subclass;

    memvol_object_t* dset_object = g_hash_table_lookup(parent->children, name);

     
    //debug Ausgaben
    if (dset_object == NULL) {
        puts("Dataset nicht im angegebenen Parent gefunden!\n");

    } else {
       memvol_dataset_t*  dset = (memvol_dataset_t *)dset_object->subclass;

//bebug
 

      DEBUG_MESSAGE("Opened dataset object %zu \n", dset_object);
      DEBUG_MESSAGE("dataset subclass %zu \n", dset_object->subclass);
      DEBUG_MESSAGE("dataset name %s \n", ((memvol_dataset_t*) dset_object->subclass)->name);
      DEBUG_MESSAGE("dataset type %zu \n", (memvol_object_type*)dset_object->type);
    }

//to do 

return (void *)dset_object;

}

static herr_t memvol_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
                   hid_t xfer_plist_id, void * buf, void **req){

puts("------------ memvol_dataset_read() called -------------\n");

//to do
return 1;
}

static herr_t memvol_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
                    hid_t xfer_plist_id, const void * buf, void **req){
puts("------------ memvol_dataset_write() called -------------\n");

memvol_dataset_t * dataset = (memvol_dataset_t*) dset;

//assert( mem_space_id <= dset.dataspace );

//number = get(mem_space_id);
//dataset->datatype* data = (dataset->datatype*)calloc(mem_space_id, sizeof(mem_type_id));

 //DEBUG_MESSAGE("data %zu\n", data);


//to do
return 1;
}

static  herr_t memvol_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){

puts("------------ memvol_dataset_get() called -------------\n");

  
   memvol_dataset_t*  dataset; 
   herr_t ret_value = 1;

   dataset = (memvol_dataset_t* )dset;

   switch(get_type) {

           case H5VL_DATASET_GET_DAPL: 
           {	
		hid_t *ret_id = va_arg (arguments, hid_t *);
                printf("Access property list %p\n", *ret_id);
                DEBUG_MESSAGE("known type found\n");
               
		break;
           }
           case H5VL_DATASET_GET_DCPL:
           {
		hid_t *ret_id = va_arg (arguments, hid_t *);
		printf("Creation property list %p\n", *ret_id);
                DEBUG_MESSAGE("known type found\n");
         
		break;

           }

	   case H5VL_DATASET_GET_OFFSET:
           {
	         haddr_t *ret = va_arg (arguments, haddr_t *);
		printf("The offset of the dataset %p \n", *ret);
		 DEBUG_MESSAGE("known type found\n");

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
                DEBUG_MESSAGE("known type found\n");

  		break;

           }
	   case H5VL_DATASET_GET_SPACE_STATUS:
	   {
		H5D_space_status_t *allocation = va_arg (arguments, H5D_space_status_t *);
		printf("Space status %p\n", *allocation);
                DEBUG_MESSAGE("known type found\n");
		break;
           }
           case H5VL_DATASET_GET_STORAGE_SIZE:
           {
		hsize_t *ret = va_arg (arguments, hsize_t *);
		printf("Storage size %p\n", *ret);
                DEBUG_MESSAGE("known type found\n");
		break;
           }
           case H5VL_DATASET_GET_TYPE:
           {
      		hid_t *ret_id = va_arg (arguments, hid_t *);
		printf("Datatype %p\n", *ret_id);
                DEBUG_MESSAGE("known type found\n");
		break;
           }
	   default:
	   {	DEBUG_MESSAGE("unknown type found\n");
		ret_value = 0;
	   }
   }
    return ret_value;       
}
 
static herr_t memvol_dataset_close(void* dset, hid_t dxpl_id, void** req) {

    puts("------------ memvol_dataset_close() called -------------\n");
  
    memvol_object_t *dataset = (memvol_object_t*) dset;
    
    free(dataset->subclass);
   

    
    dataset = NULL;
 
  //     ((memvol_dataset_t *)dataset->subclass)->name = NULL;

      

    return 1;
}
