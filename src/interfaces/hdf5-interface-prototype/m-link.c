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
// ../install/download/vol/src/H5L.c


herr_t memvol_link_create(H5VL_link_create_type_t create_type, void *obj, H5VL_loc_params_t loc_params, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id, void **req)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

herr_t memvol_link_copy(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

herr_t memvol_link_move(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

herr_t memvol_link_get(void *obj, H5VL_loc_params_t loc_params, H5VL_link_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for link GET callback */
	// typedef enum H5VL_link_get_t {
	//     H5VL_LINK_GET_INFO,        /* link info         		    */
	//     H5VL_LINK_GET_NAME,	       /* link name                         */
	//     H5VL_LINK_GET_VAL          /* link value                        */
	// } H5VL_link_get_t;

	return ret_value;
}

herr_t memvol_link_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_link_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// /* types for link SPECIFIC callback */
	// typedef enum H5VL_link_specific_t {
	//     H5VL_LINK_DELETE,          /* H5Ldelete(_by_idx)                */
	//     H5VL_LINK_EXISTS,          /* link existence                    */
	//     H5VL_LINK_ITER             /* H5Literate/visit(_by_name)              */
	// } H5VL_link_specific_t;

	return ret_value;
}

herr_t memvol_link_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments)
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// We do not define any memvol specific functionality at the moment.
	// Nothing to do.

	return ret_value;
}

