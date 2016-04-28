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

static void memvol_group_init(memvol_group_t * group){
  group->childs_tbl = g_hash_table_new (g_str_hash,g_str_equal);
  assert(group->childs_tbl != NULL);
}

static void * memvol_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    memvol_group_t *group;
    memvol_group_t *parent = (memvol_group_t *) obj;

    // check if the object exists already in the parent
    if (g_hash_table_lookup (parent->childs_tbl, name) != NULL){
      return NULL;
    }
    group = (memvol_group_t*) malloc(sizeof(memvol_group_t));
    memvol_group_init(group);

    printf("Gcreate parent %p child %p %s\n", (void*)  obj, (void*)  group, name);

    g_hash_table_insert (parent->childs_tbl, strdup(name), group);

    return (void *)group;
}

static herr_t memvol_group_close(void *grp, hid_t dxpl_id, void **req)
{
    memvol_group_t *g = (memvol_group_t *)grp;
    printf("Gclose %p\n", (void*)  g);
    return 0;
}

static void *memvol_group_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t gapl_id, hid_t dxpl_id, void **req){
  memvol_group_t *parent = (memvol_group_t *) obj;
  void * child = g_hash_table_lookup(parent->childs_tbl, name);
  printf("Gopen %p with %s child %p\n", obj, name, child);
  return child;
}

static herr_t memvol_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){
  printf("Gget %p\n", obj);
  return -1;
}

static herr_t memvol_group_specific(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments){
  printf("Gspecific %p\n", obj);
  return -1;
}

static herr_t memvol_group_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments){
  printf("Goptional %p\n", obj);
  return -1;
}
