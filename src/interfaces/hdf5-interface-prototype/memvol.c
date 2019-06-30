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


#include <assert.h>
#include <stdlib.h>

#include <glib.h>


#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <memvol-internal.h>
#include <memvol.h>

#define MEMVOL_ID 503
#define MEMVOL_NAME "h5-memvol"

// helper to inspect property lists
herr_t print_property(hid_t id, const char *name, void *iter_data) {
  debugI("%s: hid=%ld name=%s data=%p\n", __func__, id, name, iter_data);
  return 0;
}


#include "m-attribute.c"
#include "m-dataset.c"
#include "m-datatype.c"
#include "m-dummy.c"
#include "m-file.c"
#include "m-group.c"
#include "m-link.c"
#include "m-object.c"

static herr_t memvol_file_term(hid_t vtpl_id) {
  return 0;
}

static herr_t memvol_init(hid_t vipl_id) {
  memvol_init_datatype(vipl_id);
  return 0;
}


// iextract from ../install/download/vol/src/H5VLpublic.h:327
/* Class information for each VOL driver */
//typedef struct H5VL_class_t {
//    unsigned int version;                         /* Class version # */
//    H5VL_class_value_t value;                     /* value to identify plugin */
//    const char *name;                             /* Plugin name */
//    herr_t  (*initialize)(hid_t vipl_id);         /* Plugin initialization callback */
//    herr_t  (*terminate)(hid_t vtpl_id);          /* Plugin termination callback */
//    size_t  fapl_size;                            /* size of the vol info in the fapl property */
//    void *  (*fapl_copy)(const void *info);       /* callback to create a copy of the vol info */
//    herr_t  (*fapl_free)(void *info);             /* callback to release the vol info copy */
//
//    /* Data Model */
//    H5VL_attr_class_t          attr_cls;          /* attribute class callbacks */
//    H5VL_dataset_class_t       dataset_cls;       /* dataset class callbacks */
//    H5VL_datatype_class_t      datatype_cls;      /* datatype class callbacks */
//    H5VL_file_class_t          file_cls;          /* file class callbacks */
//    H5VL_group_class_t         group_cls;         /* group class callbacks */
//    H5VL_link_class_t          link_cls;          /* link class callbacks */
//    H5VL_object_class_t        object_cls;        /* object class callbacks */
//
//    /* Services */
//    H5VL_async_class_t         async_cls;         /* asynchronous class callbacks */
//    herr_t (*optional)(void *obj, hid_t dxpl_id, void **req, va_list arguments); /* Optional callback */
//} H5VL_class_t;

// TODO ignore invalid types here.
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

static const H5VL_class_t H5VL_memvol = {
0,
MEMVOL_ID,
MEMVOL_NAME,      /* name */
memvol_init,      /* initialize */
memvol_file_term, /* terminate */
sizeof(hid_t),
NULL,
NULL,
{
/* attribute_cls */
memvol_attribute_create,   /* create */
memvol_attribute_open,     /* open */
memvol_attribute_read,     /* read */
memvol_attribute_write,    /* write */
memvol_attribute_get,      /* get */
memvol_attribute_specific, /* specific */
memvol_attribute_optional, /* optional */
memvol_attribute_close     /* close */
},
{
/* dataset_cls */
memvol_dataset_create,
memvol_dataset_open,
memvol_dataset_read,
memvol_dataset_write,
memvol_dataset_get,
memvol_dataset_specific,
memvol_dataset_optional, /* optional */
memvol_dataset_close,
},
{
/* datatype_cls */
memvol_datatype_commit, /* commit */
memvol_datatype_open,   /* open */
memvol_datatype_get,    /* get_size */
NULL,                   //H5VL_log_datatype_specific,     /* specific */
NULL,                   //H5VL_log_datatype_optional,     /* optional */
memvol_datatype_close   /* close */
},
{
/* file_cls */
memvol_file_create,   /* create */
memvol_file_open,     /* open */
memvol_file_get,      /* get */
memvol_file_specific, /* specific */
memvol_file_optional, /* optional */
memvol_file_close     /* close */
},
{/* group_cls */
memvol_group_create,
memvol_group_open,
memvol_group_get,
NULL, // memvol_group_specific => Not used right now.
NULL, // memvol_group_optional => Not used right now.
memvol_group_close},
{
/* link_cls */
memvol_link_create,   /* create */
memvol_link_copy,     /* copy */
memvol_link_move,     /* move */
memvol_link_get,      /* get */
memvol_link_specific, /* specific */
memvol_link_optional, /* optional */
},
{
/* object_cls */
memvol_object_open,     /* open */
memvol_object_copy,     /* copy */
memvol_object_get,      /* get */
memvol_object_specific, /* specific */
memvol_object_optional, /* optional */
},
{/* asynchronous class callbacks */
memvol_async_cancel,
memvol_async_test,
memvol_async_wait},
NULL /* Optional callback */
};

static hid_t vol_id = -1;


hid_t H5VL_memvol_init() {
  vol_id = H5VLregister(&H5VL_memvol);
  H5VLinitialize(vol_id, H5P_DEFAULT);

  assert(H5VLget_plugin_id(MEMVOL_NAME) != -1);

  return vol_id;
}


int H5VL_memvol_finalize() {
  assert(vol_id != -1);

  H5VLclose(vol_id);
  vol_id = -1;
  return 0;
}


// see H5PL.c:695 ff for a description how the plugin is loaded.
H5PL_type_t H5PLget_plugin_type(void) {
  return H5PL_TYPE_VOL;
}

const void *H5PLget_plugin_info(void) {
  return &H5VL_memvol;
}
