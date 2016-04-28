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

static void * memvol_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name,
                      hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    memvol_t *group;
    memvol_t *o = (memvol_t *) obj;

    group = (memvol_t *)calloc(1, sizeof(memvol_t));

    return (void *)group;
}

static herr_t memvol_group_close(void *grp, hid_t dxpl_id, void **req)
{
    memvol_t *g = (memvol_t *)grp;

    free(g);
    return 0;
}

static void *memvol_group_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t gapl_id, hid_t dxpl_id, void **req){
  return NULL;
}

static herr_t memvol_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){
  return -1;
}

static herr_t memvol_group_specific(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments){
  return -1;
}

static herr_t memvol_group_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments){
  return -1;
}
