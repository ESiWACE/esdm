// This file is part of h5-memvol.
//
// This program is is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.



// extract from ../install/download/vol/src/H5Dpkg.h:435 for reference (consider any structure strictly private!)
/*
 * A dataset is made of two layers, an H5D_t struct that is unique to
 * each instance of an opened datset, and a shared struct that is only
 * created once for a given dataset.  Thus, if a dataset is opened twice,
 * there will be two IDs and two H5D_t structs, both sharing one H5D_shared_t.
 */
//typedef struct H5D_shared_t {
//    size_t              fo_count;       /* Reference count */
//    hbool_t             closing;        /* Flag to indicate dataset is closing */
//    hid_t               type_id;        /* ID for dataset's datatype    */
//    H5T_t              *type;           /* Datatype for this dataset     */
//    H5S_t              *space;          /* Dataspace of this dataset    */
//    hid_t               dcpl_id;        /* Dataset creation property id */
//    H5D_dcpl_cache_t    dcpl_cache;     /* Cached DCPL values */
//    H5O_layout_t        layout;         /* Data layout                  */
//    hbool_t             checked_filters;/* TRUE if dataset passes can_apply check */
//
//    /* Cached dataspace info */
//    unsigned            ndims;          /* The dataset's dataspace rank */
//    hsize_t             curr_dims[H5S_MAX_RANK];    /* The curr. size of dataset dimensions */
//    hsize_t             curr_power2up[H5S_MAX_RANK];    /* The curr. dim sizes, rounded up to next power of 2 */
//    hsize_t             max_dims[H5S_MAX_RANK];     /* The max. size of dataset dimensions */ 
//
//    /* Buffered/cached information for types of raw data storage*/
//    struct {
//        H5D_rdcdc_t     contig;         /* Information about contiguous data */
//                                        /* (Note that the "contig" cache
//                                         * information can be used by a chunked
//                                         * dataset in certain circumstances)
//                                         */
//        H5D_rdcc_t      chunk;          /* Information about chunked data */
//    } cache;
//
//    char                *extfile_prefix; /* expanded external file prefix */
//} H5D_shared_t;
//
//struct H5D_t {
//    H5O_loc_t           oloc;           /* Object header location       */
//    H5G_name_t          path;           /* Group hierarchy path         */
//    H5D_shared_t        *shared;        /* cached information from file */
//};



// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5D.c








static void *memvol_dataset_create(
		void *obj, 
		H5VL_loc_params_t loc_params, 
		const char *name,  
		hid_t dcpl_id, 
		hid_t dapl_id, 
		hid_t dxpl_id, 
		void **req
) {
    memvol_object_t *object;
    memvol_dataset_t *dataset;
    memvol_group_t *parent = (memvol_group_t *) ((memvol_object_t*)obj)->object;

	debugI("%s\n", __func__);

	// (gdb) bt
	// #0  memvol_dataset_create (obj=0x95f9c0, loc_params=..., name=0x961db0 "lat", dcpl_id=792633534417207319,
	//     dapl_id=792633534417207303, dxpl_id=792633534417207311, req=0x0) at /home/pq/ESiWACE/ESD-Middleware/src/m-dataset.c:86
	// #1  0x00007ffff6c7face in H5VL_dataset_create (obj=0x95f9c0, loc_params=..., vol_cls=0x95eac0, name=0x961db0 "lat",
	//     dcpl_id=792633534417207319, dapl_id=792633534417207303, dxpl_id=792633534417207311, req=0x0) at ../../src/H5VLint.c:1016
	// #2  0x00007ffff6baebeb in H5Dcreate1 (loc_id=144115188075855872, name=0x961db0 "lat", type_id=216172782113783863,
	//     space_id=288230376151711746, dcpl_id=792633534417207319) at ../../src/H5Ddeprec.c:156
	// #3  0x00007ffff74223ed in write_dim (dim=dim@entry=0x95f840, grp=grp@entry=0x966ad0, write_dimid=write_dimid@entry=NC_FALSE)
	//     at nc4hdf.c:2374
	// #4  0x00007ffff7428072 in nc4_rec_write_metadata (grp=<optimized out>, bad_coord_order=NC_FALSE) at nc4hdf.c:2563
	// #5  0x00007ffff741dad1 in sync_netcdf4_file (h5=0x95ecf0) at nc4file.c:3029
	// #6  0x00007ffff7421602 in NC4_enddef (ncid=<optimized out>) at nc4file.c:2987
	// #7  0x00007ffff73e8302 in nc_enddef (ncid=65536) at dfile.c:910
	// #8  0x0000000000400e29 in main (argc=1, argv=0x7fffffffc8e8) at /home/pq/ESiWACE/ESD-Middleware/src/tools/netcdf-bench.c:53
	// #9  0x00007ffff5cc1731 in __libc_start_main () from /lib64/libc.so.6
	// #10 0x0000000000400b69 in ?? ()



	// allocate resoources
    object  = (memvol_object_t*)  malloc(sizeof(memvol_object_t));
    dataset = (memvol_dataset_t*) malloc(sizeof(memvol_dataset_t));

	object->type = MEMVOL_DATASET;
	object->object = dataset;

    dataset->dcpl_id = H5Pcopy(dcpl_id);
    dataset->dapl_id = H5Pcopy(dapl_id);
    dataset->dxpl_id = H5Pcopy(dxpl_id);
	//dataset->loc_params = loc_params;

	hid_t spaceid;
	herr_t status = -1;

	// analyse dataspace of this dataset
	debugI("%s: spaceid=%ld \n", __func__, spaceid);
	status = H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, &spaceid);
	debugI("%s: spaceid=%ld status=%d is_simple=%d\n", __func__, spaceid, status, H5Sis_simple(spaceid));

	// get copy of space for future use (e.g. maybe required to satisfy get, specific and optional)
	dataset->dataspace = H5Scopy(spaceid);

	// analyse property lists
	size_t nprops = 0;
	void * iter_data;

	H5Pget_nprops(dcpl_id, &nprops );
    debugI("%s: dcpl_id=%ld nprops= %d \n", __func__, dcpl_id,  nprops);
	H5Piterate(dcpl_id, NULL, print_property, iter_data);

	H5Pget_nprops(dapl_id, &nprops );
    debugI("%s: dapl_id=%ld nprops= %d \n", __func__, dapl_id,  nprops);
	H5Piterate(dapl_id, NULL, print_property, iter_data);

	H5Pget_nprops(dxpl_id, &nprops );
    debugI("%s: dxpl_id=%ld nprops= %d \n", __func__, dxpl_id,  nprops);
	H5Piterate(dxpl_id, NULL, print_property, iter_data);

	// gather information about datatype
	// fetch from cpl
    debugI("%s: datatype: \n", __func__);

	// gather information about dataspace
	// fetch from cpl
    debugI("%s: dataspace: \n", __func__);
	

    if (name != NULL){ // anonymous object/datset
		// check if the object exists already in the parent
		if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
			free(dataset);
			return NULL;
			
		}
		g_hash_table_insert(parent->childs_tbl, strdup(name), object);
		g_array_append_val (parent->childs_ord_by_index_arr, object);
    }
	
    debugI("%s: Attach new dataset=(%p, %p) with name=%s to parent=%p, loc_param=%d \n", __func__, (void*) object, (void*) dataset, name, (void*) obj, loc_params.type);

	return (void *) object;
}


static void *memvol_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dapl_id, hid_t dxpl_id, void **req)
{
	debugI("%s\n", __func__);

    memvol_group_t *parent = (memvol_group_t *) ((memvol_object_t*)obj)->object;


	memvol_object_t * child = g_hash_table_lookup(parent->childs_tbl, name);
	debugI("Group open: %p with %s child %p\n", obj, name, child);

	return (void *)child;
}


static herr_t memvol_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf, void **req)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}


static herr_t memvol_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf, void **req)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}


static herr_t memvol_dataset_get(void *obj, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;
    memvol_object_t *object;
    memvol_dataset_t  *dataset;
    dataset = (memvol_dataset_t *) ((memvol_object_t*)obj)->object;


	debugI("%s: obj=%p\n", __func__, obj);

	// Variadic variables in HDF5 VOL implementation are used to expose HDF5
	// high-level calls H5*_get_*() for the various APIs through a per API 
	// single callback from within a plugins.

	// extract fromm ../install/download/vol/src/H5VLpublic.h:72
	// /* types for dataset GET callback */
	// typedef enum H5VL_dataset_get_t {
	//     H5VL_DATASET_GET_DAPL,                  /* access property list                */
	//     H5VL_DATASET_GET_DCPL,                  /* creation property list              */
	//     H5VL_DATASET_GET_OFFSET,                /* offset                              */
	//     H5VL_DATASET_GET_SPACE,                 /* dataspace                           */
	//     H5VL_DATASET_GET_SPACE_STATUS,          /* space  status                       */
	//     H5VL_DATASET_GET_STORAGE_SIZE,          /* storage size                        */
	//     H5VL_DATASET_GET_TYPE                   /* datatype                            */
	// } H5VL_dataset_get_t;

	// H5VL_DATASET_GET_SPACE:          Returns an identifier for a copy of the dataspace for a dataset.  (indeed makes a copy)
	// H5VL_DATASET_GET_SPACE_STATUS:   Determines whether space has been allocated for a dataset. 
	//  '->  return: H5D_SPACE_STATUS_NOT_ALLOCATED, H5D_SPACE_STATUS_ALLOCATED, H5D_SPACE_STATUS_PART_ALLOCATED (e.g. chunked)
	// H5VL_DATASET_GET_TYPE:           Returns an identifier for a copy of the datatype for a dataset.      
	// H5VL_DATASET_GET_DCPL:           Returns an identifier for a copy of the dataset creation property list for a dataset.
	// H5VL_DATASET_GET_DAPL:           Returns the dataset access property list associated with a dataset. 
	// H5VL_DATASET_GET_STORAGE_SIZE:   Returns the amount of storage allocated for a dataset.  
	// H5VL_DATASET_GET_OFFSET:         Returns dataset address in file. 

    switch (get_type) {
        /* H5Dget_space */
        case H5VL_DATASET_GET_SPACE:
            {
				debugI("%s: H5VL_DATASET_GET_SPACE \n", __func__);

				// va_args: &ret_value
                hid_t	*ret_id = va_arg (arguments, hid_t *);

                hid_t spaceid;
				herr_t status = -1;

				debugI("%s: spaceid=%ld \n", __func__, spaceid, spaceid);
				status = H5Pget(dataset->dcpl_id, H5VL_PROP_DSET_SPACE_ID, &spaceid);
				debugI("%s: spaceid=%ld status=%d is_simple=%d\n", __func__, spaceid, spaceid, status, H5Sis_simple(spaceid));
				debugI("%s: dataset->dataspace=%ld status=%d is_simple=%d\n", __func__, dataset->dataspace, status, H5Sis_simple(dataset->dataspace));

			
				*ret_id = H5Scopy(dataset->dataspace);
				//*ret_id = spaceid;



            	/*
                hid_t	*ret_id = va_arg (arguments, hid_t *);

                if((*ret_id = H5D_get_space(dset)) < 0)
                    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get space ID of dataset")

				*/

                break;
            }
            /* H5Dget_space_statuc */
        case H5VL_DATASET_GET_SPACE_STATUS:
            {
				debugI("%s: H5VL_DATASET_GET_SPACE_STATUS \n", __func__);

            	// var_args: allocation
                H5D_space_status_t *allocation = va_arg (arguments, H5D_space_status_t *);

            	/*
                // Read data space address and return 
                if(H5D__get_space_status(dset, allocation, dxpl_id) < 0)
                    HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to get space status")

                */

                break;
            }
            /* H5Dget_type */
        case H5VL_DATASET_GET_TYPE:
            {
				debugI("%s: H5VL_DATASET_GET_TYPE \n", __func__);

            	// va_args: &ret_value
                hid_t	*ret_id = va_arg (arguments, hid_t *);

            	/*

                if((*ret_id = H5D_get_type(dset)) < 0)
                    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get datatype ID of dataset")

                */

                break;
            }
            /* H5Dget_create_plist */
        case H5VL_DATASET_GET_DCPL:
            {
				debugI("%s: H5VL_DATASET_GET_DCPL \n", __func__);
            	
            	// va_args: &ret_value
                hid_t	*ret_id = va_arg (arguments, hid_t *);
				*ret_id = H5Pcopy(dataset->dcpl_id);
            	
            	/*
                if((*ret_id = H5D_get_create_plist(dset)) < 0)
                    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get creation property list for dataset")

                */

                break;
            }
            /* H5Dget_access_plist */
        case H5VL_DATASET_GET_DAPL:
            {
				debugI("%s: H5VL_DATASET_GET_DAPL \n", __func__);

				// va_args: &ret_value
                hid_t	*ret_id = va_arg (arguments, hid_t *);
				*ret_id = H5Pcopy(dataset->dapl_id);

            	/*
                if((*ret_id = H5D_get_access_plist(dset)) < 0)
                    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get access property list for dataset")
				*/

                break;
            }
            /* H5Dget_storage_size */
        case H5VL_DATASET_GET_STORAGE_SIZE:
            {
				debugI("%s: H5VL_DATASET_GET_STORAGE_SIZE \n", __func__);
               
               	// va_args: &ret_value
                hsize_t *ret = va_arg (arguments, hsize_t *);
            	
            	/*
                // Set return value 
                if(H5D__get_storage_size(dset, dxpl_id, ret) < 0)
                    HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, 0, "can't get size of dataset's storage")
                */

                break;
            }
            /* H5Dget_offset */
        case H5VL_DATASET_GET_OFFSET:
            {
				debugI("%s: H5VL_DATASET_GET_OFFSET \n", __func__);

				// var_args: &ret_value
                haddr_t *ret = va_arg (arguments, haddr_t *);

                /* Set return value 
                *ret = H5D__get_offset(dset);
                if(!H5F_addr_defined(*ret))
                    *ret = HADDR_UNDEF;

                */
                
                break;
            }
        default:
            //HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get this type of information from dataset")
            break;
    }


	return ret_value;
}


static herr_t memvol_dataset_specific(void *obj, H5VL_dataset_specific_t specific_type,  hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;
    memvol_object_t *object;
    memvol_dataset_t  *dataset;
    
    dataset = (memvol_dataset_t *) ((memvol_object_t*)obj)->object;

	// extract from ../install/download/vol/src/H5VLpublic.h:83
	// /* types for dataset SPECFIC callback */
	// typedef enum H5VL_dataset_specific_t {
	//     H5VL_DATASET_SET_EXTENT                 /* H5Dset_extent */
	// } H5VL_dataset_specific_t;


	// H5VL_DATASET_SET_EXTENT:    Changes the sizes of a dataset’s dimensions. 	

	switch (specific_type) {
		case H5VL_DATASET_SET_EXTENT:
		{
			debugI("%s: H5VL_DATASET_SET_EXTENT \n", __func__);
		
			const hsize_t *size = va_arg (arguments, const hsize_t *); 

			int rank;
			rank = H5Sget_simple_extent_ndims(dataset->dataspace);

			hsize_t* dims = (hsize_t*) malloc(rank * sizeof(hsize_t));
			hsize_t* max = (hsize_t*) malloc(rank * sizeof(hsize_t));

			for (int i = 0; i < rank; i++) {
				debugI("%s: rank[i]=%d, dims=%lld, max=%lld   =>   size=%lld\n", __func__, i, dims[i], max[i], size[i]);
			}
			H5Sget_simple_extent_dims(dataset->dataspace, dims, max);
			for (int i = 0; i < rank; i++) {
				debugI("%s: rank[i]=%d, dims=%lld, max=%lld   =>   size=%lld\n", __func__, i, dims[i], max[i], size[i]);
			}


			// herr_t H5Sset_extent_simple( hid_t space_id, int rank, const hsize_t *current_size, const hsize_t *maximum_size ) 
			H5Sset_extent_simple(dataset->dataspace, rank, size, max);
			
			break;
		}

		default:
			break;
	}

	return ret_value;
}



static herr_t memvol_dataset_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}


static herr_t memvol_dataset_close(void *dset, hid_t dxpl_id, void **req)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;
    memvol_dataset_t *dataset = (memvol_dataset_t *)dset;

    debugI("%s: %p\n", __func__, (void*)  dataset);

	return 0;
}
