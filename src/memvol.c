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

#include <stdlib.h>
#include <assert.h>
#include <glib.h>


#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <memvol.h>
#include <memvol-internal.h>

#define MEMVOL_ID 503
#define MEMVOL_NAME "h5-memvol"

static herr_t memvol_term(hid_t vtpl_id){
    return 0;
}

static herr_t memvol_init(hid_t vipl_id){
  return 0;
}

#include "m-attribute.c"
#include "m-dataset.c"
#include "m-datatype.c"
#include "m-file.c"
#include "m-group.c"
#include "m-link.c"
#include "m-object.c"


static const H5VL_class_t H5VL_memvol = {
    0,
    MEMVOL_ID,
    MEMVOL_NAME,
    memvol_init,                              /* initialize */
    memvol_term,                              /* terminate */
    sizeof(hid_t),
    NULL,
    NULL,
    {                                           /* attribute_cls */
        NULL, //memvol_attr_create,                /* create */
        NULL, //memvol_attr_open,                  /* open */
        NULL, //memvol_attr_read,                  /* read */
        NULL, //memvol_attr_write,                 /* write */
        NULL, //memvol_attr_get,                   /* get */
        NULL, //memvol_attr_specific,              /* specific */
        NULL, //memvol_attr_optional,              /* optional */
        NULL  //memvol_attr_close                  /* close */
    },
    {                                           /* dataset_cls */
        memvol_dataset_create,                    /* create */
        NULL,                      /* open */
        NULL,                      /* read */
        NULL,                     /* write */
        NULL, //memvol_dataset_get,               /* get */
        NULL, //memvol_dataset_specific,          /* specific */
        NULL, //memvol_dataset_optional,          /* optional */
        memvol_dataset_close                      /* close */
    },
    {                                               /* datatype_cls */
        memvol_datatype_commit,                   /* commit */
        memvol_datatype_open,                     /* open */
        memvol_datatype_get,                      /* get_size */
        NULL, //H5VL_log_datatype_specific,         /* specific */
        NULL, //H5VL_log_datatype_optional,         /* optional */
        memvol_datatype_close                     /* close */
    },
    {                                           /* file_cls */
        memvol_file_create,                      /* create */
        memvol_file_open,                        /* open */
        memvol_file_get,                         /* get */
        NULL, //memvol_file_specific,            /* specific */
        NULL, //memvol_file_optional,            /* optional */
        memvol_file_close                        /* close */
    },
    {                                           /* group_cls */
        memvol_group_create,      /* create */
        memvol_group_open,               /* open */
        memvol_group_get,                /* get */
        NULL, //memvol_group_specific,           /* specific */
        NULL, //memvol_group_optional,           /* optional */
        memvol_group_close         /* close */
    },
    {                                           /* link_cls */
        NULL, //memvol_link_create,                /* create */
        NULL, //memvol_link_copy,                  /* copy */
        NULL, //memvol_link_move,                  /* move */
        NULL, //memvol_link_get,                   /* get */
        NULL, //memvol_link_specific,              /* specific */
        NULL, //memvol_link_optional,              /* optional */
    },
    {                                           /* object_cls */
        NULL,                        /* open */
        NULL, //memvol_object_copy,                /* copy */
        NULL, //memvol_object_get,                 /* get */
        NULL,                    /* specific */
        NULL, //memvol_object_optional,            /* optional */
    },
    {
        NULL,
        NULL,
        NULL
    },
    NULL
};

static hid_t vol_id = -1;

hid_t H5VL_memvol_init(){
  vol_id = H5VLregister (& H5VL_memvol);
  H5VLinitialize(vol_id, H5P_DEFAULT);

  assert(H5VLget_plugin_id(MEMVOL_NAME) != -1);

  return vol_id;
}

int H5VL_memvol_finalize(){
  assert(vol_id != -1);

  H5VLclose(vol_id);
  vol_id = -1;
}

// see H5PL.c:695 ff for a description how the plugin is loaded.
H5PL_type_t H5PLget_plugin_type(void) {
	return H5PL_TYPE_VOL;
}

const void *H5PLget_plugin_info(void) {
	return & H5VL_memvol;
}
