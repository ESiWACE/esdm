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


static void * memvol_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    memvol_t *file;
    file = (memvol_t *)calloc(1, sizeof(memvol_t));

    return (void *)file;
}

static void * memvol_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    memvol_t *file;
    file = (memvol_t *)calloc(1, sizeof(memvol_t));

    return (void *)file;
}

static herr_t memvol_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_t *f = (memvol_t *)file;
    return 1;
}

static herr_t memvol_file_close(void *file, hid_t dxpl_id, void **req)
{
    memvol_t *f = (memvol_t *)file;
    free(f);
    return 1;
}

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
        NULL,                    /* create */
        NULL,                      /* open */
        NULL,                      /* read */
        NULL,                     /* write */
        NULL, //memvol_dataset_get,               /* get */
        NULL, //memvol_dataset_specific,          /* specific */
        NULL, //memvol_dataset_optional,          /* optional */
        NULL                      /* close */
    },
    {                                               /* datatype_cls */
      NULL,                      /* create */
      NULL,                        /* open */
      NULL,                         /* get */
      NULL, //memvol_file_specific,            /* specific */
      NULL, //memvol_file_optional,            /* optional */
      NULL                        /* close */
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
        NULL,                     /* create */
        NULL, //memvol_group_open,               /* open */
        NULL, //memvol_group_get,                /* get */
        NULL, //memvol_group_specific,           /* specific */
        NULL, //memvol_group_optional,           /* optional */
        NULL                       /* close */
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
