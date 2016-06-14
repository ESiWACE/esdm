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



static void memvol_dataset_init(memvol_dataset_t * dataset){
  //group->childs_tbl = g_hash_table_new (g_str_hash,g_str_equal);
  //group->childs_ord_by_index_arr = g_array_new(0, 0, sizeof(void*));
  //assert(group->childs_tbl != NULL);
}


void *memvol_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req)
{
    memvol_object_t *object;
    memvol_dataset_t *dataset;
    memvol_group_t *parent = (memvol_group_t *) obj;

	debugI("%s\n", __func__);

	// allocate resoources
    object  = (memvol_object_t*)  malloc(sizeof(memvol_object_t));
    dataset = (memvol_dataset_t*) malloc(sizeof(memvol_dataset_t));

	object->type = MEMVOL_DATASET;
	object->object = dataset;

    memvol_dataset_init(dataset);
    dataset->dcpl_id = H5Pcopy(dcpl_id);

    if (name != NULL){ // anonymous object/datset
		// check if the object exists already in the parent
		if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
			free(dataset);
			return NULL;
		}
		g_hash_table_insert(parent->childs_tbl, strdup(name), object);
		g_array_append_val (parent->childs_ord_by_index_arr, object);
    }
	


	return (void *)object;
}


void *memvol_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dapl_id, hid_t dxpl_id, void **req)
{
	memvol_group_t *parent = (memvol_group_t *) obj;

	debugI("%s\n", __func__);

	memvol_object_t * child = g_hash_table_lookup(parent->childs_tbl, name);
	debugI("Group open: %p with %s child %p\n", obj, name, child);

	return (void *)child->object;
}


herr_t memvol_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf, void **req)
{
	debugI("%s\n", __func__);


	debugI("%s: \n", __func__);



	return 0;
}


herr_t memvol_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf, void **req)
{
	debugI("%s\n", __func__);


	return 0;
}


herr_t memvol_dataset_get(void *obj, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	return 0;
}


herr_t memvol_dataset_specific(void *obj, H5VL_dataset_specific_t specific_type,  hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	return -1;
}


herr_t memvol_dataset_close (void *dset, hid_t dxpl_id, void **req)
{
    memvol_dataset_t *dataset = (memvol_dataset_t *)dset;

	debugI("%s\n", __func__);

    debugI("Dataset :%p\n", (void*)  dataset);

	return 0;
}
