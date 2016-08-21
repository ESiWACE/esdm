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
    int *datatype_value, *dataspace_value;


    puts("------------ memvol_dataset_create() called ------------");    
   
   memvol_dataset_t*  dataset = (memvol_dataset_t*)malloc(sizeof(memvol_dataset_t));


// name of dataset
     dataset->name = (char*)malloc(strlen(name));

// retrieving of datatype

 
    H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, datatype_value);
    dataset->datatype = (void*) malloc(sizeof(datatype_value));

// retrieving of dataspace
    H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, dataspace_value);
    dataset->dataspace = (void*) malloc(sizeof(dataspace_value));

// retrieving of link creation property list´s
//   H5Pget(dcpl_id, H5VL_DSET_LCPL_ID, *lcpl);
//   dataset->dataspace = malloc(sizeof(*lcpl));


}

static herr_t memvol_dataset_close(void* dset, hid_t dxpl_id, void** req) {

    printf("------------ memvol_group_close() called --------------");

    memvol_dataset_t *dataset = (memvol_dataset_t*) dset;

    free(dataset->name);
    free(dataset->datatype);
    free(dataset->dataspace);
    free(dataset);

    return 1;
}
