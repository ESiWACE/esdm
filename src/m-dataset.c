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

void *memvol_create(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req){
  return NULL;
}

void *memvol_open(void *obj, H5VL_loc_params_t loc_params, const char *name,  hid_t dapl_id, hid_t dxpl_id, void **req){
  return NULL;
}

herr_t memvol_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void * buf, void **req){
  return 0;
}

herr_t memvol_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void * buf, void **req){
  return 0;
}

herr_t memvol_get(void *obj, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments){
  return 0;
}

herr_t memvol_specific(void *obj, H5VL_dataset_specific_t specific_type,  hid_t dxpl_id, void **req, va_list arguments){
  return -1;
}

herr_t memvol_close (void *dset, hid_t dxpl_id, void **req){
  return -1;
}
