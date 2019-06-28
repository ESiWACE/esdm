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
 * @brief Implements the callbacks for the HDF5 VOL Plugin
 */




///////////////////////////////////////////////////////////////////////////////
// Attributes
///////////////////////////////////////////////////////////////////////////////

//
//
//            +---------------------+
//            | Primary data object |
//            +----------+----------+
//                       | 0..1
//                       |
//                       v 0..*
//               +-------+-------+
//               |   Attribute   |
//               ++-------------++
//           0..* |             | 0..*
//                |             |
//              1 v             v 1
//          +-----+----+    +---+-------+
//          | Datatype |    | Dataspace |
//          +----------+    +-----------+
//
//



// extract from ../install/download/vol/src/H5Apkg.h:233 for reference (consider any structure strictly private!)

// /* H5A routines */
// typedef struct H5VL_attr_class_t {
//     void *(*create)(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id, void **req);
//     void *(*open)(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t aapl_id, hid_t dxpl_id, void **req);
//     herr_t (*read)(void *attr, hid_t mem_type_id, void *buf, hid_t dxpl_id, void **req);
//     herr_t (*write)(void *attr, hid_t mem_type_id, const void *buf, hid_t dxpl_id, void **req);
//     herr_t (*get)(void *obj, H5VL_attr_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_loc_params_t loc_params, H5VL_attr_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*close) (void *attr, hid_t dxpl_id, void **req);
// } H5VL_attr_class_t;

// src/H5Apkg.h-94-
// /* Define the main attribute structure */
// struct H5A_t {
//     H5O_shared_t sh_loc;     /* Shared message info (must be first) */
//     H5O_loc_t    oloc;       /* Object location for object attribute is on */
//     hbool_t      obj_opened; /* Object header entry opened? */
//     H5G_name_t   path;       /* Group hierarchy path */
//     H5A_shared_t *shared;    /* Shared attribute information */
// };


// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5A.c

int mock_attr;

static void* H5VL_esdm_attribute_create (void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id, void **req) 
{
	info("%s\n", __func__);
	info("%s: Attach new attribute=TODO '%s' to obj=%p\n", __func__, attr_name, obj);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();




	size_t nprops = 0;
	void * iter_data;

	H5Pget_nprops(acpl_id, &nprops );
    info("%s: acpl_id=%ld nprops= %ld \n", __func__, acpl_id,  nprops);
	H5Piterate(acpl_id, NULL, print_property, iter_data);

	H5Pget_nprops(aapl_id, &nprops );
    info("%s: aapl_id=%ld nprops= %ld \n", __func__, aapl_id,  nprops);
	H5Piterate(aapl_id, NULL, print_property, iter_data);

	H5Pget_nprops(dxpl_id, &nprops );
    info("%s: dxpl_id=%ld nprops= %ld \n", __func__, dxpl_id,  nprops);
	H5Piterate(dxpl_id, NULL, print_property, iter_data);


//
//	hid_t type_id;
//	H5Pget(acpl_id, "attr_type_id", &type_id);
//	size_t type_size = H5Tget_size(type_id);
//	size_t data_size = type_size;
//	for(int i = 0; i < ndims; ++i) {
//		data_size *= dims[i];
//	}
//
//	switch(loc_params.obj_type)
//	{
//		case H5I_FILE:
//			info("%s: H5I_FILE \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//		case H5I_GROUP:
//			info("%s: H5I_GROUP (FALLTHROUGH)\n", __func__);
//		    /*FALLTHROUGH*/
//		case H5I_DATASET:
//			{ 
//  			info("%s: H5I_DATASET \n", __func__);
//				SQO_t* sqo = (SQO_t*) obj;
//				sqo->info.num_attrs++;
//				attribute->object.location = create_path(sqo);
//				attribute->object.name = strdup(attr_name);
//				attribute->object.root = sqo->root;
//				attribute->object.fapl = sqo->fapl;
//				attribute->data_size = data_size;
//
//				MPI_Barrier(MPI_COMM_WORLD);
//				if (0 == sqo->fapl->mpi_rank) {
//					DBA_create(attribute, loc_params, acpl_id, aapl_id, dxpl_id);
////					DBA_create(attribute, loc_params, acpl_id, aapl_id, dxpl_id);
//				}
//				MPI_Barrier(MPI_COMM_WORLD);
//			}
//			break;
//		case H5I_DATATYPE:
//			info("%s: H5I_DATATYPE \n", __func__);
//			ERRORMSG("Not implemented");
//
//			break;
//		case H5I_ATTR:
//			info("%s: H5I_ATTR \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_UNINIT:
//			info("%s: H5I_UNINIT \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_BADID:
//			info("%s: H5I_BADID \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_DATASPACE:
//			info("%s: H5I_DATASPACE \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_REFERENCE:
//			info("%s: H5I_REFRENCE \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_VFL:
//			info("%s: H5I_VFL \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_VOL:
//			info("%s: H5I_VOL \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_GENPROP_CLS:
//			info("%s: H5I_CLS \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_GENPROP_LST:
//			info("%s: H5I_LST \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_ERROR_CLASS:
//			info("%s: H5I_CLASS \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_ERROR_MSG:
//			info("%s: H5I_MSG \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_ERROR_STACK:
//			info("%s: H5I_ERROR_STACK \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		case H5I_NTYPES:
//			info("%s: H5I_NTYPES \n", __func__);
//			ERRORMSG("Not implemented");
//			break;
//
//		default:
//			ERRORMSG("Not supported");
//	} /* end switch */
//
//	return attribute;


	return (void*) &mock_attr;
}

static void* H5VL_esdm_attribute_open (
		void *obj,
		H5VL_loc_params_t loc_params,
		const char *attr_name,
		hid_t aapl_id,
		hid_t dxpl_id,
		void **req) 
{
	info("%s\n", __func__);
	info("%s: *obj = %p\n", __func__, obj);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();



	void *attribute;

	switch(loc_params.obj_type)
	{
		case H5I_GROUP:  
            // We probably can handle Group and Dataset the same way
            /*FALLTHROUGH*/
		case H5I_DATASET:
			switch (loc_params.type) {
				case H5VL_OBJECT_BY_IDX:
					{
						//DBA_open_by_idx(obj, loc_params, loc_params.loc_data.loc_by_idx.n, attribute);
						if (NULL == attribute) {
							info("Couldn't open attribute by idx");
						}
					}
					break;
				case H5VL_OBJECT_BY_NAME:
					{
						//DBA_open(obj, loc_params, attr_name, attribute);
					}
					break;
				case H5VL_OBJECT_BY_SELF:
					{
						//DBA_open(obj, loc_params, attr_name, attribute);
					}
					break;
				default:
					fail("Not supported");
			}
			break;

		case H5I_FILE:
			info("%s: H5I_FILE \n", __func__);
			break;

		case H5I_DATATYPE:
			info("%s: H5I_DATATYPE \n", __func__);
			break;

		case H5I_ATTR:
			info("%s: H5I_ATTR \n", __func__);
			break;

		case H5I_UNINIT:
			info("%s: H5I_UNINIT \n", __func__);
			break;

		case H5I_BADID:
			info("%s: H5I_BADID \n", __func__);
			break;

		case H5I_DATASPACE:
			info("%s: H5I_DATASPACE \n", __func__);
			break;

		case H5I_REFERENCE:
			info("%s: H5I_REFERENCE \n", __func__);
			break;

		case H5I_VFL:
			info("%s: H5I_VFL \n", __func__);
			break;

		case H5I_VOL:
			info("%s: H5I_VOL \n", __func__);
			break;

		case H5I_GENPROP_CLS:
			info("%s: H5I_GENPROP_CLS \n", __func__);
			break;

		case H5I_GENPROP_LST:
			info("%s: H5I_GENPROP_LST \n", __func__);
			break;

		case H5I_ERROR_CLASS:
			info("%s: H5I_ERROR_CLASS \n", __func__);
			break;

		case H5I_ERROR_MSG:
			info("%s: H5I_ERROR_MSG \n", __func__);
			break;

		case H5I_ERROR_STACK:
			info("%s: H5I_ERROR_STACK \n", __func__);
			break;

		case H5I_NTYPES:
			info("%s: H5I_NTYPES \n", __func__);
			break;

		default:
			fail("Unsupported type");
	} /* end switch */

	return (void *)attribute;



	return NULL;
}

static herr_t H5VL_esdm_attribute_read (
		void *attr,
		hid_t mem_type_id,
		void *buf,
		hid_t dxpl_id,
		void **req) 
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;


//	TRACEMSG("");
//	assert(NULL != buf);
//	SQA_t *attr = (SQA_t *)obj;
//	DBA_read(attr, buf);
//	return 1;


	return ret_value;
}

static herr_t H5VL_esdm_attribute_write (void *attr, hid_t mem_type_id, const void *buf, hid_t dxpl_id, void **req) 
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

//	SQA_t *attr = (SQA_t *)obj;
//
//	MPI_Barrier(MPI_COMM_WORLD);
//	if (0 == attr->object.fapl->mpi_rank) {
//		DBA_write(attr, buf);
//		DBA_write(attr, buf);
//	}
//	MPI_Barrier(MPI_COMM_WORLD);
//	return 1;


	return ret_value;
}

static herr_t H5VL_esdm_attribute_get (void *obj, H5VL_attr_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) 
{
	info("%s\n", __func__);
	info("%s: *obj = %p\n", __func__, obj);

    H5VL_esdm_object_t *object;
    H5VL_esdm_attribute_t *attribute;
	herr_t ret_value = SUCCEED;

	// Variadic variables in HDF5 VOL implementation are used to expose HDF5
	// high-level calls H5*_get_*() for the various APIs through a per API 
	// single callback from within a plugins.

	// extract from ../install/download/vol/src/H5VLpublic.h:54
	// /* types for attribute GET callback */
	// typedef enum H5VL_attr_get_t {
	//     H5VL_ATTR_GET_ACPL,                     /* creation property list              */
	//     H5VL_ATTR_GET_INFO,                     /* info                                */
	//     H5VL_ATTR_GET_NAME,                     /* access property list                */
	//     H5VL_ATTR_GET_SPACE,                    /* dataspace                           */
	//     H5VL_ATTR_GET_STORAGE_SIZE,             /* storage size                        */
	//     H5VL_ATTR_GET_TYPE                      /* datatype                            */
	// } H5VL_attr_get_t;

	// H5VL_ATTR_GET_SPACE:          Gets a copy of the dataspace for an attribute.   
	// H5VL_ATTR_GET_TYPE:           Gets an attribute datatype.
	// H5VL_ATTR_GET_ACPL:           Gets an attribute creation property list identifier. 
	// H5VL_ATTR_GET_NAME:           Gets an attribute name. 
	// H5VL_ATTR_GET_INFO:           Retrieves attribute information, by attribute identifier. 
	// H5VL_ATTR_GET_STORAGE_SIZE:   Returns the amount of storage required for an attribute. 

    switch (get_type) {
        case H5VL_ATTR_GET_SPACE:
            {
				info("%s: H5VL_ATTR_GET_SPACE \n", __func__);
                //hid_t	*ret_id = va_arg (arguments, hid_t *);
                //H5A_t   *attr = (H5A_t *)obj;

                //if((*ret_id = H5A_get_space(attr)) < 0)
                //    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get space ID of attribute")
                break;
            }
        /* H5Aget_type */
        case H5VL_ATTR_GET_TYPE:
            {
				info("%s: H5VL_ATTR_GET_TYPE \n", __func__);
                //hid_t	*ret_id = va_arg (arguments, hid_t *);
                //H5A_t   *attr = (H5A_t *)obj;

                //if((*ret_id = H5A_get_type(attr)) < 0)
                //    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get datatype ID of attribute")
                break;
            }
        /* H5Aget_create_plist */
        case H5VL_ATTR_GET_ACPL:
            {
				info("%s: H5VL_ATTR_GET_ACPL \n", __func__);
                //hid_t	*ret_id = va_arg (arguments, hid_t *);
                //H5A_t   *attr = (H5A_t *)obj;

                //if((*ret_id = H5A_get_create_plist(attr)) < 0)
                //    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get creation property list for attr")

                break;
            }
        /* H5Aget_name */
        case H5VL_ATTR_GET_NAME:
            {
				info("%s: H5VL_ATTR_GET_NAME \n", __func__);
                break;
            }

        /* H5Aget_info */
        case H5VL_ATTR_GET_INFO:
            {
				info("%s: H5VL_ATTR_GET_INFO \n", __func__);
				break;
            }

        case H5VL_ATTR_GET_STORAGE_SIZE:
            {
				info("%s: H5VL_ATTR_GET_STORAGE_SIZE \n", __func__);
                break;
            }

        default:
            //HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get this type of information from attr")
			fail("Cannot get this type of information from attr. %s\n", __func__);
            break;
    }

	return ret_value;
}

static herr_t H5VL_esdm_attribute_specific (void *obj, H5VL_loc_params_t loc_params, H5VL_attr_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) 
{
	info("%s\n", __func__);
	
	herr_t ret_value = SUCCEED;
	
	// /* types for attribute SPECFIC callback */
	// typedef enum H5VL_attr_specific_t {
	//     H5VL_ATTR_DELETE,                       /* H5Adelete(_by_name/idx)             */
	//     H5VL_ATTR_EXISTS,                       /* H5Aexists(_by_name)                 */
	//     H5VL_ATTR_ITER,                         /* H5Aiterate(_by_name)                */
	//     H5VL_ATTR_RENAME                        /* H5Arename(_by_name)                 */
	// } H5VL_attr_specific_t;

	// H5VL_ATTR_DELETE:     Deletes an attribute from a specified location. 
	// H5VL_ATTR_EXISTS:	 Determines whether an attribute with a given name exists on an object. 
	// H5VL_ATTR_ITER:       Calls a user’s function for each attribute on an object. 
	// H5VL_ATTR_RENAME:     Renames an attribute. 


    switch (specific_type) {

        case H5VL_ATTR_DELETE:
            {
				info("%s: H5VL_ATTR_DELETE \n", __func__);
                break;
            }

        case H5VL_ATTR_EXISTS:
            {
				info("%s: H5VL_ATTR_EXISTS \n", __func__);
                break;
            }

        case H5VL_ATTR_ITER:
            {
				info("%s: H5VL_ATTR_ITER \n", __func__);
                break;
            }

        case H5VL_ATTR_RENAME:
            {
				info("%s: H5VL_ATTR_RENAME \n", __func__);
                break;
            }

        default:
			fail("Cannot get this type of information from attr. %s\n", __func__);
        	break;
	}


	return ret_value;
}

static herr_t H5VL_esdm_attribute_optional (void *obj, hid_t dxpl_id, void **req, va_list arguments) 
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// We do not define any H5VL_esdm specific functionality at the moment.
	// Nothing to do.

	return ret_value;
}

static herr_t H5VL_esdm_attribute_close (void *attr, hid_t dxpl_id, void **req) 
{
	info("%s\n", __func__);


	// Ensure persistence
	// Free memory
	// inform ESDM, ESDM may decide to keep things cached?

	herr_t ret_value = SUCCEED;

	return ret_value;
}


///////////////////////////////////////////////////////////////////////////////
// Datatset
///////////////////////////////////////////////////////////////////////////////

// /* H5D routines */
// typedef struct H5VL_dataset_class_t {
//     void *(*create)(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req);
//     void *(*open)(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
//     herr_t (*read)(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf, void **req);
//     herr_t (*write)(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf, void **req);
//     herr_t (*get)(void *obj, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_dataset_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*close) (void *dset, hid_t dxpl_id, void **req);
// } H5VL_dataset_class_t;






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



static void *H5VL_esdm_dataset_create(
		void *obj, 
		H5VL_loc_params_t loc_params, 
		const char *name,  
		hid_t dcpl_id, 
		hid_t dapl_id, 
		hid_t dxpl_id, 
		void **req
) {
    H5VL_esdm_object_t *object;
    H5VL_esdm_dataset_t *dataset;
    H5VL_esdm_group_t *parent = (H5VL_esdm_group_t *) ((H5VL_esdm_object_t*)obj)->object;

	info("%s\n", __func__);


	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();




	// Inspect dataspace
	hid_t space_id;
	H5Pget(dcpl_id, "dataset_space_id", &space_id);
	int ndims = H5Sget_simple_extent_ndims(space_id);
	hsize_t maxdims[ndims];
	hsize_t dims[ndims];
	H5Sget_simple_extent_dims(space_id, dims, maxdims);

	
	
	hid_t type_id;
	H5Pget(dcpl_id, "dataset_type_id", &type_id);
	size_t type_size = H5Tget_size(type_id);
	size_t data_size = type_size;
	for(int i = 0; i < ndims; ++i) {
		data_size *= dims[i];
	}





	size_t height = 10;
	size_t width = 4096;





	// Interaction with ESDM
	esdm_status ret;
	esdm_container *cont = NULL;
	esdm_dataset_t *dset = NULL;


	esdm_init();


	// define dataspace
	int64_t bounds[] = {height, width};
	esdm_dataspace_t *dspace = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64);

	cont = esdm_container_create("mycontainer");
	dset = esdm_dataset_create(cont, name, dspace);

	
	esdm_container_commit(cont);
	esdm_dataset_commit(dset);



	//esdm_dataset_t* esdm_dataset_create(esdm_container *container, char * name, esdm_dataspace_t *dataspace);
	
	// (gdb) bt
	// #0  H5VL_esdm_dataset_create (obj=0x95f9c0, loc_params=..., name=0x961db0 "lat", dcpl_id=792633534417207319,
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
    object  = (H5VL_esdm_object_t*)  malloc(sizeof(H5VL_esdm_object_t));
    dataset = (H5VL_esdm_dataset_t*) malloc(sizeof(H5VL_esdm_dataset_t));

	object->type = MEMVOL_DATASET;
	object->object = dataset;

    dataset->dcpl_id = H5Pcopy(dcpl_id);
    dataset->dapl_id = H5Pcopy(dapl_id);
    dataset->dxpl_id = H5Pcopy(dxpl_id);
	//dataset->loc_params = loc_params;

	hid_t spaceid;
	herr_t status = -1;

	// analyse dataspace of this dataset
	info("%s: spaceid=%ld \n", __func__, spaceid);
	status = H5Pget(dcpl_id, H5VL_PROP_DSET_SPACE_ID, &spaceid);
	info("%s: spaceid=%ld status=%d is_simple=%d\n", __func__, spaceid, status, H5Sis_simple(spaceid));

	// get copy of space for future use (e.g. maybe required to satisfy get, specific and optional)
	dataset->dataspace = H5Scopy(spaceid);

	// analyse property lists
	size_t nprops = 0;
	void * iter_data;

	H5Pget_nprops(dcpl_id, &nprops );
    info("%s: dcpl_id=%ld nprops= %ld \n", __func__, dcpl_id,  nprops);
	H5Piterate(dcpl_id, NULL, print_property, iter_data);

	H5Pget_nprops(dapl_id, &nprops );
    info("%s: dapl_id=%ld nprops= %ld \n", __func__, dapl_id,  nprops);
	H5Piterate(dapl_id, NULL, print_property, iter_data);

	H5Pget_nprops(dxpl_id, &nprops );
    info("%s: dxpl_id=%ld nprops= %ld \n", __func__, dxpl_id,  nprops);
	H5Piterate(dxpl_id, NULL, print_property, iter_data);

	// gather information about datatype
	// fetch from cpl
    info("%s: datatype: \n", __func__);

	// gather information about dataspace
	// fetch from cpl
    info("%s: dataspace: \n", __func__);
	

    if (name != NULL){ // anonymous object/datset
		// check if the object exists already in the parent
		if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
			free(dataset);
			return NULL;
			
		}
		g_hash_table_insert(parent->childs_tbl, strdup(name), object);
		g_array_append_val (parent->childs_ord_by_index_arr, object);
    }
	
    info("%s: Attach new dataset=(%p, %p) with name=%s to parent=%p, loc_param=%d \n", __func__, (void*) object, (void*) dataset, name, (void*) obj, loc_params.type);

	return (void *) object;
}


static void *H5VL_esdm_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dapl_id, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();



    H5VL_esdm_group_t *parent = (H5VL_esdm_group_t *) ((H5VL_esdm_object_t*)obj)->object;

	//esdm_dataset_t* esdm_dataset_retrieve(esdm_container *container, const char * name);


	H5VL_esdm_object_t * child = g_hash_table_lookup(parent->childs_tbl, name);
	info("Group open: %p with %s child %p\n", obj, name, child);

	return (void *)child;
}


static herr_t H5VL_esdm_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}





static herr_t H5VL_esdm_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;




	dset_t *d = (dset_t*) malloc(sizeof(dset_t));	
	//d->object = (obj_t*) malloc(sizeof(obj_t));
	

	int ndims = H5Sget_simple_extent_ndims(mem_space_id);


	// src/H5public.h:184:typedef unsigned long long 	hsize_t;


	hsize_t dims[ndims];
	hsize_t max_dims[ndims];
	H5Sget_simple_extent_dims(mem_space_id, dims, max_dims);

	size_t block_size = H5Tget_size(mem_type_id);
	assert(block_size != 0);
	for (size_t i = 0; i < ndims; ++i) {
		block_size *= dims[i];
	}

	hsize_t start[ndims];
	hsize_t stride[ndims];
	hsize_t count[ndims];
	hsize_t block[ndims];
	if (-1 == (ret_value = H5Sget_regular_hyperslab(file_space_id, start, stride, count, block))) {
		fail("Couldn't read hyperslab.");
	}

	hid_t dspace_id;

	//unsigned char* space_buf = malloc(data_size)set pointer to data;                           
	//const void * data =  /* set pointer to data*/;                       
	//memcpy(space_buf, data, data_size);                                     
	//*space_id = H5Sdecode(space_buf);                                       
	//assert(-1 != *space_id);     
	
	hsize_t ddims[ndims];
	hsize_t dmaxdims[ndims];
	if (H5Sis_simple(dspace_id)) {
		H5Sget_simple_extent_dims(dspace_id, ddims, dmaxdims);
	}
	else {
		fail("Unsupported dataspace.");
	}


	off_t rel_offset = start[0] * block_size * d->object.fapl.mpi_size +  block_size * d->object.fapl.mpi_rank;

	off_t offset = d->offset + rel_offset;


	//dataset_update(dset, buf, block_size, offset);



	return ret_value;
}


static herr_t H5VL_esdm_dataset_get(void *obj, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;
    H5VL_esdm_object_t *object;
    H5VL_esdm_dataset_t  *dataset;
    dataset = (H5VL_esdm_dataset_t *) ((H5VL_esdm_object_t*)obj)->object;


	info("%s: obj=%p\n", __func__, obj);

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
				info("%s: H5VL_DATASET_GET_SPACE \n", __func__);

				// va_args: &ret_value
                hid_t	*ret_id = va_arg (arguments, hid_t *);

                hid_t spaceid;
				herr_t status = -1;

				info("%s: spaceid=%ld \n", __func__, spaceid, spaceid);
				status = H5Pget(dataset->dcpl_id, H5VL_PROP_DSET_SPACE_ID, &spaceid);
				info("%s: spaceid=%ld status=%d is_simple=%d\n", __func__, spaceid, spaceid, status, H5Sis_simple(spaceid));
				info("%s: dataset->dataspace=%ld status=%d is_simple=%d\n", __func__, dataset->dataspace, status, H5Sis_simple(dataset->dataspace));

			
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
				info("%s: H5VL_DATASET_GET_SPACE_STATUS \n", __func__);

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
				info("%s: H5VL_DATASET_GET_TYPE \n", __func__);

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
				info("%s: H5VL_DATASET_GET_DCPL \n", __func__);
            	
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
				info("%s: H5VL_DATASET_GET_DAPL \n", __func__);

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
				info("%s: H5VL_DATASET_GET_STORAGE_SIZE \n", __func__);
               
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
				info("%s: H5VL_DATASET_GET_OFFSET \n", __func__);

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


static herr_t H5VL_esdm_dataset_specific(void *obj, H5VL_dataset_specific_t specific_type,  hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;
    H5VL_esdm_object_t *object;
    H5VL_esdm_dataset_t  *dataset;
    
    dataset = (H5VL_esdm_dataset_t *) ((H5VL_esdm_object_t*)obj)->object;

	// extract from ../install/download/vol/src/H5VLpublic.h:83
	// /* types for dataset SPECFIC callback */
	// typedef enum H5VL_dataset_specific_t {
	//     H5VL_DATASET_SET_EXTENT                 /* H5Dset_extent */
	// } H5VL_dataset_specific_t;


	// H5VL_DATASET_SET_EXTENT:    Changes the sizes of a dataset’s dimensions. 	

	switch (specific_type) {
		case H5VL_DATASET_SET_EXTENT:
		{
			info("%s: H5VL_DATASET_SET_EXTENT \n", __func__);
		
			const hsize_t *size = va_arg (arguments, const hsize_t *); 

			int rank;
			rank = H5Sget_simple_extent_ndims(dataset->dataspace);

			hsize_t* dims = (hsize_t*) malloc(rank * sizeof(hsize_t));
			hsize_t* max = (hsize_t*) malloc(rank * sizeof(hsize_t));

			for (int i = 0; i < rank; i++) {
				info("%s: rank[i]=%d, dims=%lld, max=%lld   =>   size=%lld\n", __func__, i, dims[i], max[i], size[i]);
			}
			H5Sget_simple_extent_dims(dataset->dataspace, dims, max);
			for (int i = 0; i < rank; i++) {
				info("%s: rank[i]=%d, dims=%lld, max=%lld   =>   size=%lld\n", __func__, i, dims[i], max[i], size[i]);
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



static herr_t H5VL_esdm_dataset_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}


static herr_t H5VL_esdm_dataset_close(void *dset, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;
    H5VL_esdm_dataset_t *dataset = (H5VL_esdm_dataset_t *)dset;

    info("%s: %p\n", __func__, (void*)  dataset);

	return 0;
}





///////////////////////////////////////////////////////////////////////////////
// Datatype
///////////////////////////////////////////////////////////////////////////////

// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5T.c

// /* H5T routines*/
// typedef struct H5VL_datatype_class_t {
//     void *(*commit)(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req);
//     void *(*open)(void *obj, H5VL_loc_params_t loc_params, const char * name, hid_t tapl_id, hid_t dxpl_id, void **req);
//     herr_t (*get)   (void *obj, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_datatype_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*close) (void *dt, hid_t dxpl_id, void **req);
// } H5VL_datatype_class_t;


// TODO some locking here
static GHashTable * type_table = NULL;

static void H5VL_esdm_init_datatype(hid_t vipl_id){
  type_table =  g_hash_table_new (g_str_hash, g_str_equal);
}







static void * H5VL_esdm_datatype_t_commit(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req)
{
  g_hash_table_insert(type_table, (char*) name, (void*) type_id);
  printf("C %p\n", (void*) type_id);

  return (void*) type_id;
}

static void * H5VL_esdm_datatype_t_open(void *obj, H5VL_loc_params_t loc_params, const char * name, hid_t tapl_id, hid_t dxpl_id, void **req)
{
  void * found = g_hash_table_lookup(type_table, name);
  printf("O %p\n", found);
  hid_t tid = H5Tcreate (H5T_COMPOUND, 10);

  return (void*)tid;
}


static herr_t H5VL_esdm_datatype_t_get(void *obj, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	herr_t ret_value = SUCCEED;

	// /* types for datatype GET callback */
	// typedef enum H5VL_datatype_get_t {
	//     H5VL_DATATYPE_GET_BINARY,               /* get serialized form of transient type */ 
	//     H5VL_DATATYPE_GET_TCPL	            /* datatype creation property list	   */
	// } H5VL_datatype_get_t;

	switch (get_type) {
		case H5VL_DATATYPE_GET_BINARY:
		{ 
			// serialize datatype
			ssize_t *nalloc = va_arg (arguments, ssize_t *);
			void *buf = va_arg (arguments, void *);
			size_t size = va_arg (arguments, size_t);
			break;
		}

		case H5VL_DATATYPE_GET_TCPL:
		{ 
			// property list when the datatype has been created
			hid_t *ret_id = va_arg (arguments, hid_t *);
			*ret_id = H5P_DEFAULT;
			break;
		}

		default:
	        break;
	}

	return ret_value;
}


static herr_t H5VL_esdm_datatype_t_specific(void *obj, H5VL_datatype_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments)
{
	herr_t ret_value = SUCCEED;

	info("%s\n", __func__);

	// /* types for datatype GET callback */
	// typedef enum H5VL_datatype_get_t {
	//     H5VL_DATATYPE_GET_BINARY,               /* get serialized form of transient type */ 
	//     H5VL_DATATYPE_GET_TCPL	            /* datatype creation property list	   */
	// } H5VL_datatype_get_t;

	return ret_value;
}


static herr_t H5VL_esdm_datatype_t_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	herr_t ret_value = SUCCEED;

	info("%s\n", __func__);

	return ret_value;
}


static herr_t H5VL_esdm_datatype_t_close(void *dt, hid_t dxpl_id, void **req)
{
	herr_t ret_value = SUCCEED;

	info("%s\n", __func__);

	return ret_value;
}



///////////////////////////////////////////////////////////////////////////////
// File
///////////////////////////////////////////////////////////////////////////////

// 
// /* H5F routines */
// typedef struct H5VL_file_class_t {
//     void *(*create)(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req);
//     void *(*open)(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req);
//     herr_t (*get)(void *obj, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_file_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*close) (void *file, hid_t dxpl_id, void **req);
// } H5VL_file_class_t;
// 


// /* Structure specifically to store superblock. This was originally
//  * maintained entirely within H5F_file_t, but is now extracted
//  * here because the superblock is now handled by the cache */
// typedef struct H5F_super_t {
//     H5AC_info_t cache_info;     /* Cache entry information structure          */
//     unsigned    super_vers;     /* Superblock version                         */
//     uint8_t sizeof_addr;        /* Size of addresses in file                  */
//     uint8_t sizeof_size;        /* Size of offsets in file                    */
//     uint8_t     status_flags;   /* File status flags                          */
//     unsigned    sym_leaf_k;     /* Size of leaves in symbol tables            */
//     unsigned    btree_k[H5B_NUM_BTREE_ID]; /* B-tree key values for each type */
//     haddr_t     base_addr;      /* Absolute base address for rel.addrs.       */
//                                 /* (superblock for file is at this offset)    */
//     haddr_t     ext_addr;       /* Relative address of superblock extension   */
//     haddr_t     driver_addr;    /* File driver information block address      */
//     haddr_t     root_addr;      /* Root group address                         */
//     H5G_entry_t *root_ent;      /* Root group symbol table entry              */
// } H5F_super_t;    


// extract from ../install/download/vol/src/H5Fpkg.h:233 for reference (consider any structure strictly private!)
///*
// * Define the structure to store the file information for HDF5 files. One of
// * these structures is allocated per file, not per H5Fopen(). That is, set of
// * H5F_t structs can all point to the same H5F_file_t struct. The `nrefs'
// * count in this struct indicates the number of H5F_t structs which are
// * pointing to this struct.
// */
//struct H5F_file_t {
//    H5FD_t	*lf; 		    /* Lower level file handle for I/O	*/
//    H5F_super_t *sblock;      /* Pointer to (pinned) superblock for file */
//    H5O_drvinfo_t *drvinfo;	/* Pointer to the (pinned) driver info
//                                 * cache entry.  This field is only defined
//                                 * for older versions of the super block,
//                                 * and then only when a driver information
//                                 * block is present.  At all other times
//                                 * it should be NULL.
//                                 */
//    unsigned	nrefs;		    /* Ref count for times file is opened	*/
//    unsigned	flags;		    /* Access Permissions for file          */
//    H5F_mtab_t	mtab;		/* File mount table                     */
//    H5F_efc_t   *efc;         /* External file cache                  */
//
//    /* Cached values from FCPL/superblock */
//    uint8_t	sizeof_addr;	   /* Size of addresses in file            */
//    uint8_t	sizeof_size;	   /* Size of offsets in file              */
//    haddr_t	sohm_addr;	       /* Relative address of shared object header message table */
//    unsigned	sohm_vers;	       /* Version of shared message table on disk */
//    unsigned	sohm_nindexes;	   /* Number of shared messages indexes in the table */
//    unsigned long feature_flags; /* VFL Driver feature Flags            */
//    haddr_t	maxaddr;	       /* Maximum address for file             */
//
//    H5AC_t      *cache;		   /* The object cache	 		*/
//    H5AC_cache_config_t
//		mdc_initCacheCfg;          /* initial configuration for the      */
//                                 /* metadata cache.  This structure is   */
//                                 /* fixed at creation time and should    */
//                                 /* not change thereafter.               */
//    hid_t       fcpl_id;	       /* File creation property list ID 	*/
//    H5F_close_degree_t fc_degree;   /* File close behavior degree	*/
//    size_t	rdcc_nslots;  	   /* Size of raw data chunk cache (slots)	*/
//    size_t	rdcc_nbytes;  	   /* Size of raw data chunk cache	(bytes)	*/
//    double	rdcc_w0;	       /* Preempt read chunks first? [0.0..1.0]*/
//    size_t      sieve_buf_size;  /* Size of the data sieve buffer allocated (in bytes) */
//    hsize_t	threshold;	       /* Threshold for alignment		*/
//    hsize_t	alignment;	       /* Alignment				*/
//    unsigned	gc_ref;		       /* Garbage-collect references?		*/
//    unsigned	latest_flags;	   /* The latest version support */
//    hbool_t	store_msg_crt_idx; /* Store creation index for object header messages?	*/
//    unsigned	ncwfs;		       /* Num entries on cwfs list		*/
//    struct H5HG_heap_t **cwfs;   /* Global heap cache			*/
//    struct H5G_t *root_grp;	   /* Open root group			*/
//    H5FO_t *open_objs;           /* Open objects in file                 */
//    H5UC_t *grp_btree_shared;    /* Ref-counted group B-tree node info   */
//
//    /* File space allocation information */
//    H5F_file_space_type_t fs_strategy;	/* File space handling strategy		*/
//    hsize_t     fs_threshold;	   /* Free space section threshold 	*/
//    hbool_t     use_tmp_space;   /* Whether temp. file space allocation is allowed */
//    haddr_t	tmp_addr;          /* Next address to use for temp. space in the file */
//    unsigned fs_aggr_merge[H5FD_MEM_NTYPES];    /* Flags for whether free space can merge with aggregator(s) */
//    H5F_fs_state_t fs_state[H5FD_MEM_NTYPES];   /* State of free space manager for each type */
//    haddr_t fs_addr[H5FD_MEM_NTYPES];           /* Address of free space manager info for each type */
//    H5FS_t *fs_man[H5FD_MEM_NTYPES];            /* Free space manager for each file space type */
//    H5FD_mem_t fs_type_map[H5FD_MEM_NTYPES];    /* Mapping of "real" file space type into tracked type */
//    H5F_blk_aggr_t meta_aggr;     /* Metadata aggregation info */
//                                  /* (if aggregating metadata allocations) */
//    H5F_blk_aggr_t sdata_aggr;    /* "Small data" aggregation info */
//                                  /* (if aggregating "small data" allocations) */
//
//    /* Metadata accumulator information */
//    H5F_meta_accum_t accum;       /* Metadata accumulator info           	*/
//};
//
///*
// * This is the top-level file descriptor.  One of these structures is
// * allocated every time H5Fopen() is called although they may contain pointers
// * to shared H5F_file_t structs.
// */
//struct H5F_t {
//    char		*open_name;	                /* Name used to open file	*/
//    char		*actual_name;	            /* Actual name of the file, after resolving symlinks, etc. */
//    char               	*extpath;       /* Path for searching target external link file */
//    H5F_file_t		*shared;	        /* The shared file info		*/
//    unsigned		nopen_objs;	            /* Number of open object headers*/
//    H5FO_t              *obj_count;       /* # of time each object is opened through top file structure */
//    hbool_t             id_exists;        /* Whether an ID for this struct exists   */
//    hbool_t             closing;          /* File is in the process of being closed */
//    struct H5F_t        *parent;          /* Parent file that this file is mounted to */
//    unsigned            nmounts;          /* Number of children mounted to this file */
//#ifdef H5_HAVE_PARALLEL
//    H5P_coll_md_read_flag_t coll_md_read; /* Do all metadata reads collectively */
//    hbool_t             coll_md_write;    /* Do all metadata writes collectively */
//#endif /* H5_HAVE_PARALLEL */
//};



// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5G.c



static GHashTable * files_tbl = NULL;


static void * H5VL_esdm_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t fxpl_id, void **req)
{
    H5VL_esdm_object_t *object;
    H5VL_esdm_file_t *file;

	info("%s\n", __func__);

    info("%s: name=%s \n", __func__, name);



	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();


	// map HDF5 Files to ESDM containers
	esdm_container* cont = esdm_container_create(name);
	esdm_container_commit(cont);	// TODO: commit only after metadata was added

	// generate json object for hdf5 related container metadata
	json_error_t *error;	
	json_t * json = json_object();

	// add some metadata and commit for persistence
	json_path_set(json, "$.test.time", json_integer(21), 0, &error);
	print_json(json); // inspect
	
	//esdm_container_commit(cont);


	// analyse property lists
	size_t nprops = 0;
	void * iter_data;

	H5Pget_nprops(fcpl_id, &nprops );
    info("%s: fcpl_id=%ld nprops= %d \n", __func__, fcpl_id,  nprops);
	H5Piterate(fcpl_id, NULL, print_property, iter_data);

	H5Pget_nprops(fapl_id, &nprops );
    info("%s: fapl_id=%ld nprops= %d \n", __func__, fapl_id,  nprops);
	H5Piterate(fapl_id, NULL, print_property, iter_data);

	H5Pget_nprops(fxpl_id, &nprops );
    info("%s: fxpl_id=%ld nprops= %d \n", __func__, fxpl_id,  nprops);
	H5Piterate(fxpl_id, NULL, print_property, iter_data);


	//gchar * g_base64_encode (const guchar *data, gsize len);
	


	// create files hash map if not already existent
	if(files_tbl == NULL){
		files_tbl = g_hash_table_new (g_str_hash,g_str_equal);
	}

	// lookup the filename in the lsit of files
	file = g_hash_table_lookup (files_tbl, name);


	info("%s: files_tbl.size=%d\n", __func__, g_hash_table_size(files_tbl));


	// conform to HDF5: invalid https://www.hdfgroup.org/HDF5/doc/RM/RM_H5F.html#File-Create
	if((flags & H5F_ACC_EXCL) && file != NULL){
		return NULL;
	}

	if((flags & H5F_ACC_TRUNC) && file != NULL){
		// TODO: truncate the file. Free all structures...
		H5VL_esdm_group_init(& file->root_grp);
	}



	// create the file if not already existent
	if ( file == NULL ){

		// allocate resources
		object = (H5VL_esdm_object_t*) malloc(sizeof(H5VL_esdm_object_t));
		file = (H5VL_esdm_file_t *) malloc(sizeof(H5VL_esdm_file_t));

		// populate file and object data strutures
		H5VL_esdm_group_init(& file->root_grp);

		object->type = MEMVOL_GROUP;
		object->object =  & file->root_grp;

		g_hash_table_insert( file->root_grp.childs_tbl, strdup("/"), object);
		g_hash_table_insert( files_tbl, strdup(name), object);

		file->name = strdup(name);
		file->fcpl_id = H5Pcopy(fcpl_id);
	}

	// validate and set flags
	if( flags & H5F_ACC_RDONLY) {
		file->mode_flags = H5F_ACC_RDONLY;

	} else if (flags & H5F_ACC_RDWR) {
		file->mode_flags = H5F_ACC_RDWR;

	} else if (flags & H5F_ACC_TRUNC) {
		file->mode_flags = H5F_ACC_RDWR;

	} else {
		assert(0 && "Modeflags are invalid");
	}

	// attach to file struct
    file->mode_flags = flags;
    file->fapl_id = H5Pcopy(fapl_id);

    info("%s: New file=%p with name=%s\n", __func__, (void*) file, name);


	info("%s: files_tbl.size=%d\n", __func__, g_hash_table_size(files_tbl));


    return (void *)object;
}


static void * H5VL_esdm_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    H5VL_esdm_object_t *object;

	info("%s\n", __func__);

    info("%s: name=%s \n", __func__, name);


	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();

	//esdm_container* esdm_container_retrieve(const char * name);
	//guchar * g_base64_decode (const gchar *text, gsize *out_len);


    object = g_hash_table_lookup (files_tbl, name);

    return (void *)object;
}


static herr_t H5VL_esdm_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	herr_t ret_value = SUCCEED;
    H5VL_esdm_file_t *f = (H5VL_esdm_file_t *)file;

	info("%s\n", __func__);

	// /* types for file GET callback */
	// typedef enum H5VL_file_get_t {
	//     H5VL_FILE_GET_FAPL,                  /* file access property list	*/
	//     H5VL_FILE_GET_FCPL,	                /* file creation property list	*/
	//     H5VL_FILE_GET_INTENT,	            /* file intent           		*/
	//     H5VL_FILE_GET_NAME,	                /* file name             		*/
	//     H5VL_FILE_GET_OBJ_COUNT,	            /* object count in file	       	*/
	//     H5VL_FILE_GET_OBJ_IDS,	            /* object ids in file     		*/
	//     H5VL_OBJECT_GET_FILE                 /* retrieve or resurrect file of object */
	// } H5VL_file_get_t;

	// H5VL_FILE_GET_FAPL:            
	// H5VL_FILE_GET_FCPL:	          
	// H5VL_FILE_GET_INTENT:	      
	// H5VL_FILE_GET_NAME:	          
	// H5VL_FILE_GET_OBJ_COUNT:	      
	// H5VL_FILE_GET_OBJ_IDS:	      
	// H5VL_OBJECT_GET_FILE           


	//guchar * g_base64_decode (const gchar *text, gsize *out_len);


    switch (get_type) {
      /* H5Fget_access_plist */
      case H5VL_FILE_GET_FAPL:
      {
        hid_t *plist_id = va_arg (arguments, hid_t *);
        *plist_id = H5Pcopy(f->fapl_id);

        break;
      }
      /* H5Fget_create_plist */
      case H5VL_FILE_GET_FCPL:
      {
        hid_t *plist_id = va_arg (arguments, hid_t *);
        *plist_id = H5Pcopy(f->fcpl_id);
        break;
      }
      /* H5Fget_obj_count */
      case H5VL_FILE_GET_OBJ_COUNT:
      {
        unsigned types = va_arg (arguments, unsigned);
        ssize_t *ret = va_arg (arguments, ssize_t *);
        size_t  obj_count = 0;      /* Number of opened objects */
        assert(0 && "TODO");
        /* Set the return value */
        *ret = (ssize_t)obj_count;
        break;
      }
      /* H5Fget_obj_ids */
      case H5VL_FILE_GET_OBJ_IDS:
      {
        unsigned types = va_arg (arguments, unsigned);
        size_t max_objs = va_arg (arguments, size_t);
        hid_t *oid_list = va_arg (arguments, hid_t *);
        ssize_t *ret = va_arg (arguments, ssize_t *);
        size_t  obj_count = 0;      /* Number of opened objects */

        assert(0 && "TODO");

        /* Set the return value */
        *ret = (ssize_t)obj_count;
        break;
      }
      /* H5Fget_intent */
      case H5VL_FILE_GET_INTENT:
      {
        unsigned *ret = va_arg (arguments, unsigned *);
        *ret = f->mode_flags;
        break;
      }
      /* H5Fget_name */
      case H5VL_FILE_GET_NAME:
      {
          H5I_type_t type = va_arg (arguments, H5I_type_t);
          size_t     size = va_arg (arguments, size_t);
          char      *name = va_arg (arguments, char *);
          ssize_t   *ret  = va_arg (arguments, ssize_t *);
          size_t     len = strlen(f->name);

          if(name) {
              strncpy(name, f->name, MIN(len + 1,size));
              if(len >= size) name[size-1]='\0';
          }

          /* Set the return value for the API call */
          *ret = (ssize_t)len;
          break;
      }
      /* H5I_get_file_id */
      case H5VL_OBJECT_GET_FILE:
      {
        H5I_type_t type = va_arg (arguments, H5I_type_t);
        void ** ret = va_arg (arguments, void **);
        void * tmp;
        assert(0 && "TODO");

        switch(type) {
            case H5I_FILE:
			    info("%s: H5I_FILE \n", __func__);
                tmp = f;
                break;

            case H5I_GROUP:
			    info("%s: H5I_GROUP \n", __func__);
                break;

            case H5I_DATATYPE:
			    info("%s: H5I_DATATYPE \n", __func__);
                break;

            case H5I_DATASET:
			    info("%s: H5I_DATASET \n", __func__);
                break;

            case H5I_ATTR:
			    info("%s: H5I_ATTR \n", __func__);
                break;

            default:
                assert(0 && "Invalid datatype");
        }

        *ret = (void*) tmp;
        break;
      }
      default:
        assert(0);
    } /* end switch */

    return ret_value;
}


herr_t H5VL_esdm_file_specific(void *obj, H5VL_file_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);
	
	herr_t ret_value = SUCCEED;


//	TRACEMSG("");
//	SQF_t *file = (SQF_t *)obj;
//
//	switch(specific_type) {
//		case H5VL_FILE_FLUSH:
//			fsync(file->object.root->fd);
//			break;
//		case H5VL_FILE_IS_ACCESSIBLE:
//			ERRORMSG("Not implemented");
//			break;
//		case H5VL_FILE_MOUNT:
//			ERRORMSG("Not implemented");
//			break;
//		case H5VL_FILE_UNMOUNT:
//			ERRORMSG("Not implemented");
//			break;
//		default:
//			ERRORMSG("Uknown type");
//	}
//

    return ret_value;
}


herr_t H5VL_esdm_file_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);
	
	herr_t ret_value = SUCCEED;

    return ret_value;
}


static herr_t H5VL_esdm_file_close(void *file, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);
	
	herr_t ret_value = SUCCEED;

    return ret_value;
}







///////////////////////////////////////////////////////////////////////////////
// Group
///////////////////////////////////////////////////////////////////////////////

// /* H5G routines */
// typedef struct H5VL_group_class_t {
//     void *(*create)(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req);
//     void *(*open)(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gapl_id, hid_t dxpl_id, void **req);
//     herr_t (*get)(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*close) (void *grp, hid_t dxpl_id, void **req);
// } H5VL_group_class_t;

// extract from ../install/download/vol/src/H5Gpkg.h:138 for reference (consider any structure strictly private!)
/*
 * Shared information for all open group objects
 */
//struct H5G_shared_t {
//    int fo_count;                   /* open file object count */
//    hbool_t mounted;                /* Group is mount point */
//};
//
/*
 * A group handle passed around through layers of the library within and
 * above the H5G layer.
 */
//struct H5G_t {
//    H5G_shared_t *shared;               /* Shared file object data */
//    H5O_loc_t oloc;                     /* Object location for group */
//    H5G_name_t path;                    /* Group hierarchy path   */
//};

// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5G.c


static void H5VL_esdm_group_init(H5VL_esdm_group_t * group)
{
  group->childs_tbl = g_hash_table_new (g_str_hash,g_str_equal);
  group->childs_ord_by_index_arr = g_array_new(0, 0, sizeof(void*));
  assert(group->childs_tbl != NULL);
}


static void * H5VL_esdm_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    H5VL_esdm_object_t *object;
    H5VL_esdm_group_t  *group;
    H5VL_esdm_group_t *parent = (H5VL_esdm_group_t *) ((H5VL_esdm_object_t*)obj)->object;

	info("%s\n", __func__);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();



	//esdm_container* esdm_container_create(const char *name);

	// allocate resources
    object = (H5VL_esdm_object_t*) malloc(sizeof(H5VL_esdm_object_t));
    group  = (H5VL_esdm_group_t*)  malloc(sizeof(H5VL_esdm_group_t));

	object->type = MEMVOL_GROUP;
	object->object = group;

    H5VL_esdm_group_init(group);
    group->gcpl_id = H5Pcopy(gcpl_id);

    info("%s: Attach new group=(%p, %p) with name=%s to parent=%p, loc_param=%d \n", __func__, (void*) object, (void*) group, name, (void*) obj, loc_params.type);

    if (name != NULL){ // anonymous object/group
      // check if the object exists already in the parent
      if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
        free(group);
        return NULL;
      }
      g_hash_table_insert(parent->childs_tbl, strdup(name), object);
      g_array_append_val (parent->childs_ord_by_index_arr, object);

    }

    return (void *)object;
}




static void *H5VL_esdm_group_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t gapl_id, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();

	
	// map HDF5 group to ESDM containers
	esdm_container* cont = esdm_container_create(name);
	esdm_container_commit(cont);	// TODO: commit only after metadata was added

	// generate json object for hdf5 related container metadata
	json_error_t *error;	
	json_t * json = json_object();

	// add some metadata and commit for persistence
	json_path_set(json, "$.test.time", json_integer(21), 0, &error);
	print_json(json); // inspect
	
	esdm_container_commit(cont);



    H5VL_esdm_group_t *parent = (H5VL_esdm_group_t *) ((H5VL_esdm_object_t*)obj)->object;

	H5VL_esdm_object_t * child = g_hash_table_lookup(parent->childs_tbl, name);
	info("%s: Found group=%p with name=%s in parent=%p\n", __func__, child->object, name, obj);


	//esdm_container* esdm_container_retrieve(const char * name);


	return (void *)child;
}



static herr_t H5VL_esdm_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;
	H5VL_esdm_group_t *group = (H5VL_esdm_group_t *) ((H5VL_esdm_object_t*)obj)->object;

	// Variadic variables in HDF5 VOL implementation are used to expose HDF5
	// high-level calls H5*_get_*() for the various APIs through a single
	// callback from wihtin the plugins.

	// /* types for group GET callback */
	// typedef enum H5VL_group_get_t {
	//     H5VL_GROUP_GET_GCPL,	            /* group creation property list	*/
	//     H5VL_GROUP_GET_INFO 	            /* group info             		*/
	// } H5VL_group_get_t;

	switch (get_type) {
		case H5VL_GROUP_GET_GCPL:
			{
				// group creation property list (GCPL)
				info("Group get: GCPL %p\n", obj);
				hid_t *new_gcpl_id = va_arg(arguments, hid_t *);
				*new_gcpl_id = H5Pcopy(group->gcpl_id);
				return 0;
			}

		case H5VL_GROUP_GET_INFO:{

			// This argument defines if we should retrieve information about ourselve or a child node
			H5VL_loc_params_t loc_params = va_arg(arguments, H5VL_loc_params_t);
			H5G_info_t *grp_info         = va_arg(arguments, H5G_info_t *);

			info("Group get: INFO %p loc_param: %d \n", obj, loc_params.type);

			H5VL_esdm_group_t * relevant_group;



			if(loc_params.type == H5VL_OBJECT_BY_SELF) {
				relevant_group = group;

			} else if (loc_params.type == H5VL_OBJECT_BY_NAME) {
				relevant_group = g_hash_table_lookup(group->childs_tbl, loc_params.loc_data.loc_by_name.name);
				if (relevant_group == NULL){
					return -1;
				}
				relevant_group = (H5VL_esdm_group_t*) ((H5VL_esdm_object_t*)relevant_group)->object;

			} else if (loc_params.type == H5VL_OBJECT_BY_IDX) {
				assert(loc_params.loc_data.loc_by_idx.order == H5_ITER_INC || loc_params.loc_data.loc_by_idx.order == H5_ITER_NATIVE);
				if(loc_params.loc_data.loc_by_idx.idx_type == H5_INDEX_NAME){
					// TODO, for now return the index position.
					relevant_group = g_array_index(group->childs_ord_by_index_arr, H5VL_esdm_group_t*, loc_params.loc_data.loc_by_idx.n);
					relevant_group = (H5VL_esdm_group_t*) ((H5VL_esdm_object_t*)relevant_group)->object;

				} else if(loc_params.loc_data.loc_by_idx.idx_type == H5_INDEX_CRT_ORDER){
					relevant_group = g_array_index(group->childs_ord_by_index_arr, H5VL_esdm_group_t*, loc_params.loc_data.loc_by_idx.n);
					relevant_group = (H5VL_esdm_group_t*) ((H5VL_esdm_object_t*)relevant_group)->object;

				} else {
					assert(0);
				}
			}

			grp_info->storage_type = H5G_STORAGE_TYPE_COMPACT;
			grp_info->nlinks = 0;
			grp_info->max_corder = g_hash_table_size(relevant_group->childs_tbl);
			grp_info->mounted = 0;

			return 0;
		}
	}
	
    return ret_value;
}



static herr_t H5VL_esdm_group_specific(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for group SPECFIC callback */
	// typedef enum H5VL_group_specific_t {
	//     H5VL_GROUP_SPECIFIC_INVALID
	// } H5VL_group_specific_t;

    return ret_value;
}



static herr_t H5VL_esdm_group_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// We do not define any H5VL_esdm specific functionality at the moment.
	// Nothing to do.

    return ret_value;
}





static herr_t H5VL_esdm_group_close(void *grp, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;
	H5VL_esdm_group_t *group = (H5VL_esdm_group_t *) ((H5VL_esdm_object_t*)grp)->object;

    info("Group close: %p\n", (void*) group);

    return ret_value;
}






///////////////////////////////////////////////////////////////////////////////
// Link
///////////////////////////////////////////////////////////////////////////////

// 
// /* H5L routines */
// typedef struct H5VL_link_class_t {
//     herr_t (*create)(H5VL_link_create_type_t create_type, void *obj, H5VL_loc_params_t loc_params, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id, void **req);
//     herr_t (*copy)(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req);
//     herr_t (*move)(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req);
//     herr_t (*get)(void *obj, H5VL_loc_params_t loc_params, H5VL_link_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_loc_params_t loc_params, H5VL_link_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
// } H5VL_link_class_t;
// 



// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5L.c


herr_t H5VL_esdm_link_create(H5VL_link_create_type_t create_type, void *obj, H5VL_loc_params_t loc_params, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();


	herr_t ret_value = SUCCEED;

	return ret_value;
}

herr_t H5VL_esdm_link_copy(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

herr_t H5VL_esdm_link_move(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

herr_t H5VL_esdm_link_get(void *obj, H5VL_loc_params_t loc_params, H5VL_link_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for link GET callback */
	// typedef enum H5VL_link_get_t {
	//     H5VL_LINK_GET_INFO,        /* link info         		    */
	//     H5VL_LINK_GET_NAME,	       /* link name                         */
	//     H5VL_LINK_GET_VAL          /* link value                        */
	// } H5VL_link_get_t;

	return ret_value;
}

herr_t H5VL_esdm_link_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_link_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for link SPECIFIC callback */
	// typedef enum H5VL_link_specific_t {
	//     H5VL_LINK_DELETE,          /* H5Ldelete(_by_idx)                */
	//     H5VL_LINK_EXISTS,          /* link existence                    */
	//     H5VL_LINK_ITER             /* H5Literate/visit(_by_name)              */
	// } H5VL_link_specific_t;

	return ret_value;
}

herr_t H5VL_esdm_link_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// We do not define any H5VL_esdm specific functionality at the moment.
	// Nothing to do.

	return ret_value;
}




///////////////////////////////////////////////////////////////////////////////
// Object
///////////////////////////////////////////////////////////////////////////////






// /* H5O routines */
// typedef struct H5VL_object_class_t {
//     void *(*open)(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req);
//     herr_t (*copy)(void *src_obj, H5VL_loc_params_t loc_params1, const char *src_name, void *dst_obj, H5VL_loc_params_t loc_params2, const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req);
//     herr_t (*get)(void *obj, H5VL_loc_params_t loc_params, H5VL_object_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*specific)(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//     herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments);
// } H5VL_object_class_t;
// 



// Extract from : src/H5Opkg.h:263:struct H5O_t {

// struct H5O_t {
//     H5AC_info_t cache_info; /* Information for metadata cache functions, _must_ be */
//                             /* first field in structure */
// 
//     /* File-specific information (not stored) */
//     size_t      sizeof_size;            /* Size of file sizes            */
//     size_t      sizeof_addr;            /* Size of file addresses        */
// 
//     /* Debugging information (not stored) */
// #ifdef H5O_ENABLE_BAD_MESG_COUNT
//     hbool_t     store_bad_mesg_count;   /* Flag to indicate that a bad message count should be stored */
//                                         /* (This is to simulate a bug in earlier
//                                          *      versions of the library)
//                                          */
// #endif /* H5O_ENABLE_BAD_MESG_COUNT */
// #ifndef NDEBUG
//     size_t      ndecode_dirtied;        /* Number of messages dirtied by decoding */
// #endif /* NDEBUG */
// 
//     /* Chunk management information (not stored) */
//     size_t      rc;                     /* Reference count of [continuation] chunks using this structure */
//     size_t      chunk0_size;            /* Size of serialized first chunk    */
//     hbool_t     mesgs_modified;         /* Whether any messages were modified when the object header was deserialized */
//     hbool_t     prefix_modified;        /* Whether prefix was modified when the object header was deserialized */
// 
//     /* Object information (stored) */
//     hbool_t     has_refcount_msg;       /* Whether the object has a ref. count message */
//     unsigned    nlink;          /*link count                 */
//     uint8_t version;        /*version number             */
//     uint8_t flags;          /*flags                  */
// 
//     /* Time information (stored, for versions > 1 & H5O_HDR_STORE_TIMES flag set) */
//     time_t      atime;                  /*access time                */
//     time_t      mtime;                  /*modification time              */
//     time_t      ctime;                  /*change time                */
//     time_t      btime;                  /*birth time                 */
// 
//     /* Attribute information (stored, for versions > 1) */
//     unsigned    max_compact;        /* Maximum # of compact attributes   */
//     unsigned    min_dense;      /* Minimum # of "dense" attributes   */
// 
//     /* Message management (stored, encoded in chunks) */
//     size_t  nmesgs;         /*number of messages             */
//     size_t  alloc_nmesgs;       /*number of message slots        */
//     H5O_mesg_t  *mesg;          /*array of messages          */
//     size_t      link_msgs_seen;         /* # of link messages seen when loading header */
//     size_t      attr_msgs_seen;         /* # of attribute messages seen when loading header */
// 
//     /* Chunk management (not stored) */
//     size_t  nchunks;        /*number of chunks           */
//     size_t  alloc_nchunks;      /*chunks allocated           */
//     H5O_chunk_t *chunk;         /*array of chunks            */
// };
// 
// /* Class for types of objects in file */
// typedef struct H5O_obj_class_t {
//     H5O_type_t  type;               /*object type on disk        */
//     const char  *name;              /*for debugging          */
//     void       *(*get_copy_file_udata)(void);   /*retrieve user data for 'copy file' operation */
//     void    (*free_copy_file_udata)(void *); /*free user data for 'copy file' operation */
//     htri_t  (*isa)(H5O_t *);        /*if a header matches an object class */
//     hid_t   (*open)(const H5G_loc_t *, hid_t, hid_t, hbool_t ); /*open an object of this class */
//     void    *(*create)(H5F_t *, void *, H5G_loc_t *, hid_t );   /*create an object of this class */
//     H5O_loc_t   *(*get_oloc)(hid_t );       /*get the object header location for an object */
//     herr_t      (*bh_info)(const H5O_loc_t *loc, hid_t dxpl_id, H5O_t *oh, H5_ih_info_t *bh_info); /*get the index & heap info for an object */
//     herr_t      (*flush)(void *obj_ptr, hid_t dxpl_id); /*flush an opened object of this class */
// } H5O_obj_class_t;   


// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5G.c



void * H5VL_esdm_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	// ensure ESDM initialized for HDF5 API entry points (H5*open, H5*create)
	esdm_init();


	return NULL;
}


herr_t H5VL_esdm_object_copy(void *src_obj, H5VL_loc_params_t loc_params1, const char *src_name, void *dst_obj, H5VL_loc_params_t loc_params2, const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}


herr_t H5VL_esdm_object_get(void *obj, H5VL_loc_params_t loc_params, H5VL_object_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for object GET callback */
	// typedef enum H5VL_object_get_t {
	//     H5VL_REF_GET_NAME,                 /* object name                       */
	//     H5VL_REF_GET_REGION,               /* dataspace of region               */
	//     H5VL_REF_GET_TYPE                  /* type of object                    */
	// } H5VL_object_get_t;


	return ret_value;
}


herr_t H5VL_esdm_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for object SPECIFIC callback */
	// typedef enum H5VL_object_specific_t {
	//     H5VL_OBJECT_CHANGE_REF_COUNT,      /* H5Oincr/decr_refcount              */
	//     H5VL_OBJECT_EXISTS,                /* H5Oexists_by_name                  */
	//     H5VL_OBJECT_VISIT,                 /* H5Ovisit(_by_name)                 */
	//     H5VL_REF_CREATE                    /* H5Rcreate                          */
	// } H5VL_object_specific_t;


	

    switch (specific_type) {

        case H5VL_OBJECT_CHANGE_REF_COUNT:
            {
				info("%s: H5VL_OBJECT_CHANGE_REF_COUNT \n", __func__);
                break;
            }

        case H5VL_OBJECT_EXISTS:
            {
				info("%s: H5VL_OBJECT_EXISTS \n", __func__);
                break;
            }

        case H5VL_OBJECT_VISIT:
            {
				info("%s: H5VL_OBJECT_VISIT \n", __func__);
                break;
            }

        case H5VL_REF_CREATE:
            {
				info("%s: H5VL_REF_CREATE \n", __func__);
                break;
            }

        default:
        	break;
	}




	return ret_value;

}


herr_t H5VL_esdm_object_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	info("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// We do not define any H5VL_esdm specific functionality at the moment.
	// Nothing to do.
	
	return ret_value;
}




///////////////////////////////////////////////////////////////////////////////
// Async
///////////////////////////////////////////////////////////////////////////////

// /* H5AO routines */
// typedef struct H5VL_async_class_t {
//     herr_t (*cancel)(void **, H5ES_status_t *);
//     herr_t (*test)  (void **, H5ES_status_t *);
//     herr_t (*wait)  (void **, H5ES_status_t *);
// } H5VL_async_class_t;




herr_t H5VL_esdm_async_cancel(){
	info("%s (NOT IMPLEMENTED)\n", __func__);


	herr_t ret_value = SUCCEED;
	return ret_value;
}

herr_t H5VL_esdm_async_test(){
	info("%s (NOT IMPLEMENTED)\n", __func__);

	herr_t ret_value = SUCCEED;
	return ret_value;
}

herr_t H5VL_esdm_async_wait(){
	info("%s (NOT IMPLEMENTED)\n", __func__);

	herr_t ret_value = SUCCEED;
	return ret_value;
}


