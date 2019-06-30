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
 * @brief HDF5 Virtual Object Layer Plugin providing ESDM Support
 */

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

#include <assert.h>
#include <glib.h>
#include <hdf5.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "esdm.h"
#include "h5-esdm.h"

#define VOL_PLUGIN_ID 71
#define VOL_PLUGIN_NAME "h5-esdm"

// helper to inspect property lists
herr_t print_property(hid_t id, const char *name, void *iter_data) {
  info("%s: hid=%ld name=%s data=%p\n", __func__, id, name, iter_data);
  return 0;
}

// Declaration of callbacks for VOL functions. Forward declarations ommitted
// on purpose to avoid changes at multiple locations.
#include "h5-esdm-callbacks.c"

// Forward declaration of init and terminate.
herr_t H5VL_esdm_init(hid_t vipl_id);
int H5VL_esdm_term();

/**
 * Populate a HDF5 VOL object with plugin metadata and callbacks prior to
 * registration with HDF5.
 */
static const H5VL_class_t H5VL_esdm = {
0,               // unsigned int version;                      /* Class version # */
VOL_PLUGIN_ID,   // H5VL_class_value_t value;                  /* value to identify plugin */
VOL_PLUGIN_NAME, // const char *name;                          /* Plugin name */
H5VL_esdm_init,  // herr_t  (*initialize)(hid_t vipl_id);      /* Plugin initialization callback */
H5VL_esdm_term,  // herr_t  (*terminate)(hid_t vtpl_id);       /* Plugin termination callback */
sizeof(hid_t),   // size_t  fapl_size;                         /* size of the vol info in the fapl property */
NULL,            // void *  (*fapl_copy)(const void *info);    /* callback to create a copy of the vol info */
NULL,            // herr_t  (*fapl_free)(void *info);          /* callback to release the vol info copy */

{
/* attribute_cls */
H5VL_esdm_attribute_create,   /* create */
H5VL_esdm_attribute_open,     /* open */
H5VL_esdm_attribute_read,     /* read */
H5VL_esdm_attribute_write,    /* write */
H5VL_esdm_attribute_get,      /* get */
H5VL_esdm_attribute_specific, /* specific */
H5VL_esdm_attribute_optional, /* optional */
H5VL_esdm_attribute_close     /* close */
},
{
/* dataset_cls */
H5VL_esdm_dataset_create,
H5VL_esdm_dataset_open,
H5VL_esdm_dataset_read,
H5VL_esdm_dataset_write,
H5VL_esdm_dataset_get,
H5VL_esdm_dataset_specific,
H5VL_esdm_dataset_optional, /* optional */
H5VL_esdm_dataset_close,
},
{
/* type_cls */
H5VL_esdm_type_t_commit, /* commit */
H5VL_esdm_type_t_open,   /* open */
H5VL_esdm_type_t_get,    /* get_size */
NULL,                    //H5VL_log_type_specific,        /* specific */
NULL,                    //H5VL_log_type_optional,        /* optional */
H5VL_esdm_type_t_close   /* close */
},
{
/* file_cls */
H5VL_esdm_file_create,   /* create */
H5VL_esdm_file_open,     /* open */
H5VL_esdm_file_get,      /* get */
H5VL_esdm_file_specific, /* specific */
H5VL_esdm_file_optional, /* optional */
H5VL_esdm_file_close     /* close */
},
{/* group_cls */
H5VL_esdm_group_create,
H5VL_esdm_group_open,
H5VL_esdm_group_get,
NULL, // H5VL_esdm_group_specific => Not used right now.
NULL, // H5VL_esdm_group_optional => Not used right now.
H5VL_esdm_group_close},
{
/* link_cls */
H5VL_esdm_link_create,   /* create */
H5VL_esdm_link_copy,     /* copy */
H5VL_esdm_link_move,     /* move */
H5VL_esdm_link_get,      /* get */
H5VL_esdm_link_specific, /* specific */
H5VL_esdm_link_optional, /* optional */
},
{
/* object_cls */
H5VL_esdm_object_open,     /* open */
H5VL_esdm_object_copy,     /* copy */
H5VL_esdm_object_get,      /* get */
H5VL_esdm_object_specific, /* specific */
H5VL_esdm_object_optional, /* optional */
},
{/* asynchronous class callbacks */
H5VL_esdm_async_cancel,
H5VL_esdm_async_test,
H5VL_esdm_async_wait},
NULL /* Optional callback */
};

static hid_t vol_id = -1;

herr_t H5VL_esdm_init(hid_t vipl_id) {
  info("H5VL_esdm_init()");

  vol_id = H5VLregister(&H5VL_esdm);
  H5VLinitialize(vol_id, H5P_DEFAULT);

  assert(H5VLget_plugin_id(VOL_PLUGIN_NAME) != -1);

  esdm_init();

  return (herr_t)vol_id;
}

int H5VL_esdm_term() {
  info("H5VL_esdm_term()");

  assert(vol_id != -1);

  H5VLclose(vol_id);
  vol_id = -1;
  return 0;
}

// see H5PL.c:695 for a description how the plugin is loaded.
H5PL_type_t H5PLget_plugin_type(void) {
  return H5PL_TYPE_VOL;
}

const void *H5PLget_plugin_info(void) {
  return &H5VL_esdm;
}
