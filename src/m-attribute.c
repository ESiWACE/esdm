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



// extract from ../install/download/vol/src/H5Apkg.h:233 for reference (consider any structure strictly private!)



// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5A.c

int puipui;

static void* memvol_attribute_create (void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id, void **req) 
{
	debugI("%s\n", __func__);
	debugI("%s: Attach new attribute=TODO '%s' to obj=%p\n", __func__, attr_name, obj) 

	return (void*) &puipui;
}

static void* memvol_attribute_open (void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t aapl_id, hid_t dxpl_id, void **req) 
{
	debugI("%s\n", __func__);
	debugI("%s: *obj = %p\n", __func__, obj) 

	return NULL;
}

static herr_t memvol_attribute_read (void *attr, hid_t mem_type_id, void *buf, hid_t dxpl_id, void **req) 
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

static herr_t memvol_attribute_write (void *attr, hid_t mem_type_id, const void *buf, hid_t dxpl_id, void **req) 
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}

static herr_t memvol_attribute_get (void *obj, H5VL_attr_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) 
{
	debugI("%s\n", __func__);
	debugI("%s: *obj = %p\n", __func__, obj) 

    memvol_object_t *object;
    memvol_attribute_t *attribute;
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
				debugI("%s: H5VL_ATTR_GET_SPACE \n", __func__);
                //hid_t	*ret_id = va_arg (arguments, hid_t *);
                //H5A_t   *attr = (H5A_t *)obj;

                //if((*ret_id = H5A_get_space(attr)) < 0)
                //    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get space ID of attribute")
                break;
            }
        /* H5Aget_type */
        case H5VL_ATTR_GET_TYPE:
            {
				debugI("%s: H5VL_ATTR_GET_TYPE \n", __func__);
                //hid_t	*ret_id = va_arg (arguments, hid_t *);
                //H5A_t   *attr = (H5A_t *)obj;

                //if((*ret_id = H5A_get_type(attr)) < 0)
                //    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get datatype ID of attribute")
                break;
            }
        /* H5Aget_create_plist */
        case H5VL_ATTR_GET_ACPL:
            {
				debugI("%s: H5VL_ATTR_GET_ACPL \n", __func__);
                //hid_t	*ret_id = va_arg (arguments, hid_t *);
                //H5A_t   *attr = (H5A_t *)obj;

                //if((*ret_id = H5A_get_create_plist(attr)) < 0)
                //    HGOTO_ERROR(H5E_ARGS, H5E_CANTGET, FAIL, "can't get creation property list for attr")

                break;
            }
        /* H5Aget_name */
        case H5VL_ATTR_GET_NAME:
            {
				debugI("%s: H5VL_ATTR_GET_NAME \n", __func__);
                break;
            }

        /* H5Aget_info */
        case H5VL_ATTR_GET_INFO:
            {
				debugI("%s: H5VL_ATTR_GET_INFO \n", __func__);
				break;
            }

        case H5VL_ATTR_GET_STORAGE_SIZE:
            {
				debugI("%s: H5VL_ATTR_GET_STORAGE_SIZE \n", __func__);
                break;
            }

        default:
            //HGOTO_ERROR(H5E_VOL, H5E_CANTGET, FAIL, "can't get this type of information from attr")
            break;
    }

	return ret_value;
}

static herr_t memvol_attribute_specific (void *obj, H5VL_loc_params_t loc_params, H5VL_attr_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) 
{
	debugI("%s\n", __func__);
	
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
				debugI("%s: H5VL_ATTR_DELETE \n", __func__);
                break;
            }

        case H5VL_ATTR_EXISTS:
            {
				debugI("%s: H5VL_ATTR_EXISTS \n", __func__);
                break;
            }

        case H5VL_ATTR_ITER:
            {
				debugI("%s: H5VL_ATTR_ITER \n", __func__);
                break;
            }

        case H5VL_ATTR_RENAME:
            {
				debugI("%s: H5VL_ATTR_RENAME \n", __func__);
                break;
            }

        default:
        	break;
	}


	return ret_value;
}

static herr_t memvol_attribute_optional (void *obj, hid_t dxpl_id, void **req, va_list arguments) 
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	// We do not define any memvol specific functionality at the moment.
	// Nothing to do.

	return ret_value;
}

static herr_t memvol_attribute_close (void *attr, hid_t dxpl_id, void **req) 
{
	debugI("%s\n", __func__);

	herr_t ret_value = SUCCEED;

	return ret_value;
}
