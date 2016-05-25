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

static void memvol_dataset_init(memvol_dataset_t * dataset){
  //group->childs_tbl = g_hash_table_new (g_str_hash,g_str_equal);
  //group->childs_ord_by_index_arr = g_array_new(0, 0, sizeof(void*));
  //assert(group->childs_tbl != NULL);
}


void *memvol_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req){
	debugI("%s\n", __func__);

    memvol_dataset_t *dataset;
    memvol_group_t *parent = (memvol_group_t *) obj;

    dataset = (memvol_dataset_t*) malloc(sizeof(memvol_dataset_t));
    memvol_dataset_init(dataset);
    dataset->dcpl_id = H5Pcopy(dcpl_id);

    //if (name != NULL){ // anonymous group
    //  // check if the object exists already in the parent
    //  if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
    //    free(group);
    //    return NULL;
    //  }
    //  g_hash_table_insert(parent->childs_tbl, strdup(name), group);
    //  g_array_append_val (parent->childs_ord_by_index_arr, group);
    //}
	


	return (void *)dataset;
}

void *memvol_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dapl_id, hid_t dxpl_id, void **req){
	debugI("%s\n", __func__);

	/*
	memvol_group_t *parent = (memvol_group_t *) obj;
	void * child = g_hash_table_lookup(parent->childs_tbl, name);
	debugI("Gopen %p with %s child %p\n", obj, name, child);
	return child;
	*/

	return NULL;
}

herr_t memvol_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf, void **req){
	debugI("%s\n", __func__);

	return 0;
}

herr_t memvol_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf, void **req){
	debugI("%s\n", __func__);

	return 0;
}

herr_t memvol_dataset_get(void *obj, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){
	debugI("%s\n", __func__);

	return 0;
}

herr_t memvol_dataset_specific(void *obj, H5VL_dataset_specific_t specific_type,  hid_t dxpl_id, void **req, va_list arguments){
	debugI("%s\n", __func__);

	return -1;
}

herr_t memvol_dataset_close (void *dset, hid_t dxpl_id, void **req){
	debugI("%s\n", __func__);

	return -1;
}
