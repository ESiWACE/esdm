// This file is part of h5-memvol.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5G.c

void *memvol_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req) {
  debugI("%s\n", __func__);
  return NULL;
}

herr_t memvol_object_copy(void *src_obj, H5VL_loc_params_t loc_params1, const char *src_name, void *dst_obj, H5VL_loc_params_t loc_params2, const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req) {
  debugI("%s\n", __func__);

  herr_t ret_value = SUCCEED;

  return ret_value;
}

herr_t memvol_object_get(void *obj, H5VL_loc_params_t loc_params, H5VL_object_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  debugI("%s\n", __func__);

  herr_t ret_value = SUCCEED;

  // /* types for object GET callback */
  // typedef enum H5VL_object_get_t {
  //     H5VL_REF_GET_NAME,                 /* object name                       */
  //     H5VL_REF_GET_REGION,               /* dataspace of region               */
  //     H5VL_REF_GET_TYPE                  /* type of object                    */
  // } H5VL_object_get_t;

  return ret_value;
}

herr_t memvol_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) {
  debugI("%s\n", __func__);

  herr_t ret_value = SUCCEED;

  // /* types for object SPECIFIC callback */
  // typedef enum H5VL_object_specific_t {
  //     H5VL_OBJECT_CHANGE_REF_COUNT,      /* H5Oincr/decr_refcount              */
  //     H5VL_OBJECT_EXISTS,                /* H5Oexists_by_name                  */
  //     H5VL_OBJECT_VISIT,                 /* H5Ovisit(_by_name)                 */
  //     H5VL_REF_CREATE                    /* H5Rcreate                          */
  // } H5VL_object_specific_t;

  switch (specific_type) {
    case H5VL_OBJECT_CHANGE_REF_COUNT: {
      debugI("%s: H5VL_OBJECT_CHANGE_REF_COUNT \n", __func__);
      break;
    }

    case H5VL_OBJECT_EXISTS: {
      debugI("%s: H5VL_OBJECT_EXISTS \n", __func__);
      break;
    }

    case H5VL_OBJECT_VISIT: {
      debugI("%s: H5VL_OBJECT_VISIT \n", __func__);
      break;
    }

    case H5VL_REF_CREATE: {
      debugI("%s: H5VL_REF_CREATE \n", __func__);
      break;
    }

    default:
      break;
  }

  return ret_value;
}

herr_t memvol_object_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments) {
  debugI("%s\n", __func__);

  herr_t ret_value = SUCCEED;

  // We do not define any memvol specific functionality at the moment.
  // Nothing to do.

  return ret_value;
}
