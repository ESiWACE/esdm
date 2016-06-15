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



static void memvol_group_init(memvol_group_t * group)
{
  group->childs_tbl = g_hash_table_new (g_str_hash,g_str_equal);
  group->childs_ord_by_index_arr = g_array_new(0, 0, sizeof(void*));
  assert(group->childs_tbl != NULL);
}


static void * memvol_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    memvol_object_t *object;
    memvol_group_t  *group;
    memvol_group_t *parent = (memvol_group_t *) ((memvol_object_t*)obj)->object;

	debugI("%s\n", __func__);

	

	// allocate resources
    object = (memvol_object_t*) malloc(sizeof(memvol_object_t));
    group  = (memvol_group_t*)  malloc(sizeof(memvol_group_t));

	object->type = MEMVOL_GROUP;
	object->object = group;

    memvol_group_init(group);
    group->gcpl_id = H5Pcopy(gcpl_id);

    debugI("%s: Attach new group=%p with name=%s to parent=%p, loc_param=%d \n", __func__, (void*) group, name, (void*) obj, loc_params.type);

    if (name != NULL){ // anonymous object/group
      // check if the object exists already in the parent
      if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
        free(group);
        return NULL;
      }
      g_hash_table_insert(parent->childs_tbl, strdup(name), object);
      g_array_append_val (parent->childs_ord_by_index_arr, object);

      //g_hash_table_insert(parent->childs_tbl, strdup(name), group);
      //g_array_append_val (parent->childs_ord_by_index_arr, group);
    }

    return (void *)object;
}


static void *memvol_group_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t gapl_id, hid_t dxpl_id, void **req)
{
    memvol_group_t *parent = (memvol_group_t *) ((memvol_object_t*)obj)->object;

	debugI("%s\n", __func__);

	memvol_object_t * child = g_hash_table_lookup(parent->childs_tbl, name);
	debugI("%s: Found group=%p with name=%s in parent=%p\n", __func__, child->object, name, obj);


	return (void *)child;
}


static herr_t memvol_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
  debugI("%s\n", __func__);

  memvol_group_t *g = (memvol_group_t *) ((memvol_object_t*)obj)->object;

  switch (get_type) {
    case H5VL_GROUP_GET_GCPL:{
    	// group creation property list (GCPL)
        debugI("Group get: GCPL %p\n", obj);
		hid_t *new_gcpl_id = va_arg(arguments, hid_t *);
		*new_gcpl_id = H5Pcopy(g->gcpl_id);
		return 0;
    }

    case H5VL_GROUP_GET_INFO:{

        // This argument defines if we should retrieve information about ourselve or a child node
        H5VL_loc_params_t loc_params = va_arg(arguments, H5VL_loc_params_t);
        H5G_info_t *grp_info         = va_arg(arguments, H5G_info_t *);

        debugI("Group get: INFO %p loc_param: %d \n", obj, loc_params.type);

        memvol_group_t * relevant_group;



        if(loc_params.type == H5VL_OBJECT_BY_SELF) {
            relevant_group = g;

        } else if (loc_params.type == H5VL_OBJECT_BY_NAME) {
            relevant_group = g_hash_table_lookup(g->childs_tbl, loc_params.loc_data.loc_by_name.name);
            if (relevant_group == NULL){
                return -1;
            }
            relevant_group = (memvol_group_t*) ((memvol_object_t*)relevant_group)->object;

        } else if (loc_params.type == H5VL_OBJECT_BY_IDX) {
          assert(loc_params.loc_data.loc_by_idx.order == H5_ITER_INC || loc_params.loc_data.loc_by_idx.order == H5_ITER_NATIVE);
          if(loc_params.loc_data.loc_by_idx.idx_type == H5_INDEX_NAME){
              // TODO, for now return the index position.
              relevant_group = g_array_index(g->childs_ord_by_index_arr, memvol_group_t*, loc_params.loc_data.loc_by_idx.n);
              relevant_group = (memvol_group_t*) ((memvol_object_t*)relevant_group)->object;

          } else if(loc_params.loc_data.loc_by_idx.idx_type == H5_INDEX_CRT_ORDER){
              relevant_group = g_array_index(g->childs_ord_by_index_arr, memvol_group_t*, loc_params.loc_data.loc_by_idx.n);
              relevant_group = (memvol_group_t*) ((memvol_object_t*)relevant_group)->object;

          } else{
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
  return -1;
}


static herr_t memvol_group_close(void *obj, hid_t dxpl_id, void **req)
{
	memvol_group_t *g = (memvol_group_t *) ((memvol_object_t*)obj)->object;

    debugI("Group close: %p\n", (void*)  g);

    return 0;
}
