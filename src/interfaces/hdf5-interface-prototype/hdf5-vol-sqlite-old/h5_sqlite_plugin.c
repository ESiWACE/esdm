
//Autor Olga

// Author: Eugen Betke

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <hdf5.h>
#include <libgen.h> // basename
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "base.h"
#include "db_iface.h"
#include "debug.h"
#include "h5_sqlite_plugin.h"

#ifdef ADAPTIVE
#  include "esdm.h"
#endif /* adaptive */


#define COUNT_MAX 2147479552
#define SSDDIR "/tmp"
#define SHMDIR "/dev/shm"


static herr_t H5VL_extlog_fapl_free(void *info);
static void *H5VL_extlog_fapl_copy(const void *info);


static herr_t H5VL_log_init(hid_t vipl_id);
static herr_t H5VL_log_term(hid_t vtpl_id);

/* Atrribute callbacks */
static void *H5VL_extlog_attr_create(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id, void **req);
static void *H5VL_extlog_attr_open(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t aapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_attr_read(void *attr, hid_t dtype_id, void *buf, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_attr_write(void *attr, hid_t dtype_id, const void *buf, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_attr_get(void *obj, H5VL_attr_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_attr_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_attr_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_attr_close(void *attr, hid_t dxpl_id, void **req);

/* Datatype callbacks */
static void *H5VL_extlog_datatype_commit(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req);
static void *H5VL_extlog_datatype_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t tapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_datatype_get(void *dt, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_datatype_close(void *dt, hid_t dxpl_id, void **req);

/* Dataset callbacks */
static void *H5VL_extlog_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req);
static void *H5VL_extlog_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t plist_id, void *buf, void **req);
static herr_t H5VL_extlog_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t plist_id, const void *buf, void **req);
static herr_t H5VL_extlog_dataset_close(void *dset, hid_t dxpl_id, void **req);

/* File callbacks */
static void *H5VL_extlog_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req);
static void *H5VL_extlog_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_file_close(void *file, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_file_specific(void *obj, H5VL_file_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);


/* Group callbacks */
static void *H5VL_extlog_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req);
static void *H5VL_extlog_group_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_group_close(void *grp, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_group_specific(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_group_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments);


/* Link callbacks */
static herr_t H5VL_extlog_link_create(H5VL_link_create_type_t create_type, void *obj, H5VL_loc_params_t loc_params, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_link_copy(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_link_move(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_link_get(void *obj, H5VL_loc_params_t loc_params, H5VL_link_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_link_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_link_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_link_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments);


/* H5O routines */
static void *H5VL_extlog_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_object_copy(void *src_obj, H5VL_loc_params_t loc_params1, const char *src_name, void *dst_obj, H5VL_loc_params_t loc_params2, const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_extlog_object_get(void *obj, H5VL_loc_params_t loc_params, H5VL_object_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_extlog_object_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments);


///* Object callbacks */
//static void *H5VL_extlog_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req);
//static herr_t H5VL_extlog_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);
//
hid_t native_plugin_id = -1;
haddr_t addr_g         = 0;
size_t counter_g       = 0;

static const H5VL_class_t H5VL_log_g = {
0,
LOG,
"extlog",      /* name */
H5VL_log_init, /* initialize */
H5VL_log_term, /* terminate */
sizeof(h5sqlite_fapl_t),
H5VL_extlog_fapl_copy,
H5VL_extlog_fapl_free,
{
/* attribute_cls */
H5VL_extlog_attr_create,   /* create */
H5VL_extlog_attr_open,     /* open */
H5VL_extlog_attr_read,     /* read */
H5VL_extlog_attr_write,    /* write */
H5VL_extlog_attr_get,      /* get */
H5VL_extlog_attr_specific, /* specific */
NULL,                      //H5VL_extlog_attr_optional,              /* optional */
H5VL_extlog_attr_close     /* close */
},
{
/* dataset_cls */
H5VL_extlog_dataset_create, /* create */
H5VL_extlog_dataset_open,   /* open */
H5VL_extlog_dataset_read,   /* read */
H5VL_extlog_dataset_write,  /* write */
H5VL_extlog_dataset_get,    /* get */
NULL,                       //H5VL_extlog_dataset_specific,          /* specific */
NULL,                       //H5VL_extlog_dataset_optional,          /* optional */
H5VL_extlog_dataset_close   /* close */
},
{
/* datatype_cls */
H5VL_extlog_datatype_commit, /* commit */
H5VL_extlog_datatype_open,   /* open */
H5VL_extlog_datatype_get,    /* get_size */
NULL,                        //H5VL_extlog_datatype_specific,         /* specific */
NULL,                        //H5VL_extlog_datatype_optional,         /* optional */
H5VL_extlog_datatype_close   /* close */
},
{
/* file_cls */
H5VL_extlog_file_create,   /* create */
H5VL_extlog_file_open,     /* open */
H5VL_extlog_file_get,      /* get */
H5VL_extlog_file_specific, /* specific */
NULL,                      //H5VL_extlog_file_optional,            /* optional */
H5VL_extlog_file_close     /* close */
},
{
/* group_cls */
H5VL_extlog_group_create,   /* create */
H5VL_extlog_group_open,     /* open */
H5VL_extlog_group_get,      /* get */
H5VL_extlog_group_specific, /* specific */
H5VL_extlog_group_optional, /* optional */
H5VL_extlog_group_close     /* close */
},
{
/* link_cls */
H5VL_extlog_link_create,   /* create */
H5VL_extlog_link_copy,     /* copy */
H5VL_extlog_link_move,     /* move */
H5VL_extlog_link_get,      /* get */
H5VL_extlog_link_specific, /* specific */
H5VL_extlog_link_optional, /* optional */
},
{
/* object_cls */
H5VL_extlog_object_open,     /* open */
H5VL_extlog_object_copy,     /* copy */
H5VL_extlog_object_get,      /* get */
H5VL_extlog_object_specific, /* specific */
H5VL_extlog_object_optional, /* optional */
},
{NULL,
NULL,
NULL},
NULL};


static void SQO_init_info(H5O_info_t *info) {
  info->fileno                    = 0;
  info->addr                      = 0;
  info->type                      = H5O_TYPE_UNKNOWN;
  info->rc                        = 0;
  info->atime                     = 0;
  info->mtime                     = 0;
  info->ctime                     = 0;
  info->btime                     = 0;
  info->num_attrs                 = 0;
  info->hdr.version               = 0; /* Version number of header format in file  */
  info->hdr.nmesgs                = 0; /* Number of object header messages         */
  info->hdr.nchunks               = 0; /* Number of object header chunks           */
  info->hdr.flags                 = 0; /* Object header status flags               */
  info->hdr.space.total           = 0; /* Total space for storing object header in */
  info->hdr.space.meta            = 0; /* Space within header for object header    */
  info->hdr.space.mesg            = 0; /* Space within header for actual message   */
  info->hdr.space.free            = 0; /* Free space within object header          */
  info->hdr.mesg.present          = 0; /* Flags to indicate presence of message    */
  info->hdr.mesg.shared           = 0; /* Flags to indicate message type is        */
  info->meta_size.obj.heap_size   = 0;
  info->meta_size.obj.index_size  = 0;
  info->meta_size.attr.heap_size  = 0;
  info->meta_size.attr.index_size = 0;
}


//char *err_msg = 0;  /* pointer to an error string */

//h5sqlite_fapl_t* ginfo = NULL;

static void *H5VL_extlog_fapl_copy(const void *info) {
  const h5sqlite_fapl_t *fapl_source = (h5sqlite_fapl_t *)info;
  h5sqlite_fapl_t *fapl_target       = (h5sqlite_fapl_t *)malloc(sizeof(*fapl_target));
  memcpy(fapl_target, fapl_source, sizeof(fapl_source));
  fapl_target->fn      = strdup(fapl_source->fn);
  fapl_target->db_fn   = strdup(fapl_source->db_fn);
  fapl_target->data_fn = strdup(fapl_source->data_fn);
  //	fapl_target->offset = fapl_source->offset;
  return (void *)fapl_target;
}

static herr_t H5VL_extlog_fapl_free(void *info) {
  herr_t err            = 0;
  h5sqlite_fapl_t *fapl = (h5sqlite_fapl_t *)info;
  free(fapl->fn);
  free(fapl->db_fn);
  free(fapl->data_fn);
  return err;
}


static herr_t H5VL_log_init(hid_t vipl_id) {
  TRACEMSG("");
  native_plugin_id = H5VLget_plugin_id("native");
  assert(native_plugin_id > 0);
  printf("------- LOG INIT\n");
  return 0;
}

static herr_t H5VL_log_term(hid_t vtpl_id) {
  DEBUGMSG("------- LOG TERM\n");
  return 0;
}


static herr_t
print_property(hid_t id, const char *name, void *iter_data) {
  DEBUGMSG("%s: hid=%ld name=%s data=%p\n", __func__, id, name, iter_data);
  return 0;
}

/*-------------------------------------------------------------------------
 * Function:	H5VL_extlog_attr_create
 *
 * Purpose:	Creates an attribute on an object.
 *-------------------------------------------------------------------------
 */

static void *
H5VL_extlog_attr_create(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  SQA_t *attribute = (SQA_t *)malloc(sizeof(*attribute));

  hid_t space_id;
  H5Pget(acpl_id, "attr_space_id", &space_id);
  int ndims = H5Sget_simple_extent_ndims(space_id);
  hsize_t maxdims[ndims];
  hsize_t dims[ndims];
  H5Sget_simple_extent_dims(/* in */ space_id, /* out */ dims, /* out */ maxdims);


  hid_t type_id;
  H5Pget(acpl_id, "attr_type_id", &type_id);
  size_t type_size = H5Tget_size(type_id);
  size_t data_size = type_size;
  for (int i = 0; i < ndims; ++i) {
    data_size *= dims[i];
  }

  switch (loc_params.obj_type) {
    case H5I_FILE:
      ERRORMSG("Not implemented");
      break;
    case H5I_GROUP:
    case H5I_DATASET: {
      SQO_t *sqo = (SQO_t *)obj;
      sqo->info.num_attrs++;
      attribute->object.location = create_path(sqo);
      attribute->object.name     = strdup(attr_name);
      attribute->object.root     = sqo->root;
      attribute->object.fapl     = sqo->fapl;
      attribute->data_size       = data_size;

      MPI_Barrier(MPI_COMM_WORLD);
      if (0 == sqo->fapl->mpi_rank) {
        DBA_create(attribute, loc_params, acpl_id, aapl_id, dxpl_id);
        //					DBA_create(attribute, loc_params, acpl_id, aapl_id, dxpl_id);
      }
      MPI_Barrier(MPI_COMM_WORLD);
    } break;
    case H5I_DATATYPE:
      ERRORMSG("Not implemented");
      break;
    case H5I_ATTR:
      ERRORMSG("Not implemented");
      break;
    case H5I_UNINIT:
      ERRORMSG("Not implemented");
      break;
    case H5I_BADID:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATASPACE:
      ERRORMSG("Not implemented");
      break;
    case H5I_REFERENCE:
      ERRORMSG("Not implemented");
      break;
    case H5I_VFL:
      ERRORMSG("Not implemented");
      break;
    case H5I_VOL:
      ERRORMSG("Not implemented");
      break;
    case H5I_GENPROP_CLS:
      ERRORMSG("Not implemented");
      break;
    case H5I_GENPROP_LST:
      ERRORMSG("Not implemented");
      break;
    case H5I_ERROR_CLASS:
      ERRORMSG("Not implemented");
      break;
    case H5I_ERROR_MSG:
      ERRORMSG("Not implemented");
      break;
    case H5I_ERROR_STACK:
      ERRORMSG("Not implemented");
      break;
    case H5I_NTYPES:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Not supported");
  } /* end switch */

  return attribute;
}


/*-------------------------------------------------------------------------
 * Function:	H5VL_extlog_attr_open
 *
 * Purpose:	Opens a attr inside a native h5 file
 *-------------------------------------------------------------------------
 */

static void *
H5VL_extlog_attr_open(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t aapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  SQA_t *attribute = (SQA_t *)malloc(sizeof(*attribute));
  SQO_t *sqo       = (SQO_t *)obj;

  switch (loc_params.obj_type) {
    case H5I_GROUP:
    case H5I_DATASET:
      switch (loc_params.type) {
        case H5VL_OBJECT_BY_IDX: {
          DBA_open_by_idx(obj, loc_params, loc_params.loc_data.loc_by_idx.n, attribute);
          if (NULL == attribute) {
            ERRORMSG("Couldn't open attribute by idx");
          }
        } break;
        case H5VL_OBJECT_BY_NAME: {
          DBA_open(obj, loc_params, attr_name, attribute);
        } break;
        case H5VL_OBJECT_BY_SELF: {
          DBA_open(obj, loc_params, attr_name, attribute);
        } break;
        default:
          ERRORMSG("Not supported");
      }
      break;
    case H5I_FILE:
    case H5I_DATATYPE:
    case H5I_ATTR:
      break;
    case H5I_UNINIT:
    case H5I_BADID:
    case H5I_DATASPACE:
    case H5I_REFERENCE:
    case H5I_VFL:
    case H5I_VOL:
    case H5I_GENPROP_CLS:
    case H5I_GENPROP_LST:
    case H5I_ERROR_CLASS:
    case H5I_ERROR_MSG:
    case H5I_ERROR_STACK:
    case H5I_NTYPES:
      ERRORMSG("Not implemented");
    default:
      ERRORMSG("unsupported type");
  } /* end switch */

  return (void *)attribute;
}

/*-------------------------------------------------------------------------
 * Function:	H5VL_extlog_attr_read
 *
 * Purpose:	Reads in data from attribute
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL_extlog_attr_read(void *obj, hid_t dtype_id, void *buf, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  assert(NULL != buf);
  SQA_t *attr = (SQA_t *)obj;
  DBA_read(attr, buf);
  return 1;
}

/*-------------------------------------------------------------------------
 * Function:	H5VL_extlog_attr_write
 *
 * Purpose:	Writes out data to attribute
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL_extlog_attr_write(void *obj, hid_t dtype_id, const void *buf, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  SQA_t *attr = (SQA_t *)obj;

  MPI_Barrier(MPI_COMM_WORLD);
  if (0 == attr->object.fapl->mpi_rank) {
    DBA_write(attr, buf);
    //		DBA_write(attr, buf);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  return 1;
}


/*-------------------------------------------------------------------------
 * Function:	H5VL_extlog_attr_get
 *
 * Purpose:	Gets certain information about an attribute
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL_extlog_attr_get(void *obj, H5VL_attr_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  //	SQA_t* attr = (SQA_t*) obj;
  herr_t ret_value = 0;
  switch (get_type) {
    case H5VL_ATTR_GET_ACPL:
      ERRORMSG("Not implemented");
      break;
    case H5VL_ATTR_GET_INFO:
      ERRORMSG("Not implemented");
      break;
    case H5VL_ATTR_GET_NAME: {
      H5VL_loc_params_t loc_params = va_arg(arguments, H5VL_loc_params_t);
      size_t buf_size              = va_arg(arguments, size_t);
      char *buf                    = va_arg(arguments, char *);
      ssize_t *ret_val             = va_arg(arguments, ssize_t *);
      switch (loc_params.type) {
        case H5VL_OBJECT_BY_SELF: {
          SQA_t *sqa = (SQA_t *)obj;
          strcpy(buf, sqa->object.name);
          *ret_val = strlen(sqa->object.name);
        } break;
        case H5VL_OBJECT_BY_IDX:
          ERRORMSG("Not implemented");
          break;
        default:
          ERRORMSG("Not implemented");
      }
    } break;
    case H5VL_ATTR_GET_SPACE: {
      hid_t *ret_id = va_arg(arguments, hid_t *);
      DBA_get_space(obj, ret_id);
      assert(-1 != *ret_id);
    } break;
    case H5VL_ATTR_GET_STORAGE_SIZE:
      ERRORMSG("Not implemented");
      break;
    case H5VL_ATTR_GET_TYPE: {
      hid_t *ret_id = va_arg(arguments, hid_t *);
      DBA_get_type(obj, ret_id);
      assert(-1 != *ret_id);
    } break;
    default:
      ERRORMSG("Not supported");
  }

  //ret_value = H5VLattr_get(o->under_object, native_plugin_id, get_type, dxpl_id, req, arguments);
  return ret_value;
}


static htri_t
SQA_exists_by_self(void *obj, H5VL_loc_params_t loc_params, const char *attr_name, hid_t dxpl_id) {
  TRACEMSG("");
  htri_t ret_val = false;
  switch (loc_params.obj_type) {
    case H5I_GROUP:
    case H5I_FILE:
    case H5I_DATATYPE:
    case H5I_DATASPACE:
    case H5I_DATASET: {
      SQO_t *sqo = (SQO_t *)obj;
      int exists = 0;
      DB_entry_exists(sqo, "ATTRIBUTES", attr_name, &exists);
      if (1 == exists) {
        ret_val = true;
      }
    } break;
    default:
      ERRORMSG("Not implemented");
  }
  return ret_val;
}


static herr_t SQA_iterate(
SQO_t *obj,
H5VL_loc_params_t loc_params,
H5_index_t idx_type,
H5_iter_order_t order,
hsize_t *idx,
H5A_operator2_t op,
void *op_data,
hid_t dxpl_id) {
  SQO_t *obj2  = NULL;
  hid_t vol_id = H5VLget_plugin_id("extlog");
  assert(-1 != vol_id);
  hid_t loc_id = -1;

  switch (loc_params.obj_type) {
    case H5I_GROUP: {
      SQG_t *sqg = (SQG_t *)malloc(sizeof(*sqg));
      memcpy(sqg, obj, sizeof(SQG_t));
      obj2 = (SQO_t *)sqg;
    } break;
    case H5I_DATASET: {
      SQD_t *sqd = (SQD_t *)malloc(sizeof(*sqd));
      memcpy(sqd, obj, sizeof(SQD_t));
      obj2 = (SQO_t *)sqd;
    } break;
    case H5I_FILE:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATATYPE:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATASPACE:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Not supported");
  }

  obj2->fapl       = obj->fapl;
  obj2->root       = obj->root;
  obj2->location   = strdup(obj->location);
  obj2->name       = strdup(obj->name);
  obj2->info.btime = ++counter_g;

  loc_id = H5VLobject_register(obj2, loc_params.obj_type, vol_id);
  assert(-1 != loc_id);

  switch (idx_type) {
    case H5_INDEX_NAME: //An alpha-numeric index by attribute name
      switch (order) {
        case H5_ITER_INC:
          switch (loc_params.obj_type) {
            case H5I_GROUP:
            case H5I_FILE:
            case H5I_DATATYPE:
            case H5I_DATASPACE:
            case H5I_DATASET: {
              H5A_info_t ainfo;
              char **attr_list      = NULL;
              size_t attr_list_size = 0;
              DB_create_name_list(obj, loc_params, "ATTRIBUTES", &attr_list, &attr_list_size);

              for (size_t i = 0; i < attr_list_size; ++i) {
                DBA_get_info(obj, attr_list[i], &ainfo);
                op(loc_id, attr_list[i], &ainfo, op_data);
              }

              DB_destroy_name_list(attr_list, attr_list_size);
            } break;
            default:
              ERRORMSG("Not implemented");
          }


          break;
        case H5_ITER_DEC:
          ERRORMSG("Not implemented");
          break;
        case H5_ITER_NATIVE:
          ERRORMSG("Not implemented");
          break;
      }
      break;
    case H5_INDEX_CRT_ORDER: //An index by creation order
      switch (order) {
        case H5_ITER_INC:
          ERRORMSG("Not implemented");
          break;
        case H5_ITER_DEC:
          ERRORMSG("Not implemented");
          break;
        case H5_ITER_NATIVE:
          ERRORMSG("Not implemented");
          break;
        default:
          ERRORMSG("Not implemented");
      }
      break;
    default:
      ERRORMSG("Not implemented");
  }
  return 0;
}


static herr_t H5VL_extlog_attr_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_attr_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  htri_t *ret_id;
  switch (specific_type) {
    case H5VL_ATTR_DELETE:
      TODOMSG("Not implemented H5VL_ATTR_DELETE");
      break;
    case H5VL_ATTR_EXISTS: {
      const char *attr_name = va_arg(arguments, const char *);
      ret_id                = va_arg(arguments, htri_t *);
      switch (loc_params.type) {
        case H5VL_OBJECT_BY_SELF: {
          SQO_t *sqo = (SQO_t *)obj;
          *ret_id    = SQA_exists_by_self(obj, loc_params, attr_name, dxpl_id);
        } break;
        case H5VL_OBJECT_BY_NAME:
          ERRORMSG("Not implemented");
          break;
        default:
          ERRORMSG("Unknown type");
      }
      break;
    }
    case H5VL_ATTR_ITER: {
      H5_index_t idx_type   = va_arg(arguments, H5_index_t);
      H5_iter_order_t order = va_arg(arguments, H5_iter_order_t);
      hsize_t *idx          = va_arg(arguments, hsize_t *);
      H5A_operator2_t op    = va_arg(arguments, H5A_operator2_t);
      void *op_data         = va_arg(arguments, void *);

      SQA_iterate(obj, loc_params, idx_type, order, idx, op, op_data, dxpl_id);
    } break;
    case H5VL_ATTR_RENAME:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Uknown type");
  }

  return 0;
}


/*-------------------------------------------------------------------------
 * Function:	H5VL_extlog_attr_close
 *
 * Purpose:	Closes an attribute
 *-------------------------------------------------------------------------
 */
static herr_t
H5VL_extlog_attr_close(void *attr, hid_t dxpl_id, void **req) {
  //	TRACEMSG("");
  SQA_t *a = (SQA_t *)attr;
  free(a->object.location);
  a->object.location = NULL;
  free(a->object.name);
  a->object.name = NULL;
  free(a);
  a = NULL;
  return 0;
}


char *real_filename_create(h5sqlite_fapl_t *fapl) {
  char rank_buf[50];
  sprintf(rank_buf, "%d", fapl->mpi_rank);
  char *fname = malloc(strlen(fapl->data_fn) + strlen(rank_buf) + 1);
  strcpy(fname, fapl->data_fn);
  strcat(fname, rank_buf);
  return fname;
}

void real_filename_destroy(char *fname) {
  free(fname);
  fname = NULL;
}


static void *
H5VL_extlog_file_create(const char *fname, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");

  int err;
  SQF_t *file           = (SQF_t *)malloc(sizeof(*file));
  h5sqlite_fapl_t *fapl = (h5sqlite_fapl_t *)H5VL_extlog_fapl_copy(H5Pget_vol_info(fapl_id));

  MPI_Comm_rank(MPI_COMM_WORLD, &fapl->mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &fapl->mpi_size);


  if (access(fapl->db_fn, F_OK) != -1) {
    if (H5F_ACC_TRUNC == flags) {
      if (0 == fapl->mpi_rank) {
        err = unlink(fapl->db_fn);
        if (0 != err) {
          ERRORMSG("Couldn't delete %s", fapl->db_fn);
        }
        if (access(fapl->data_fn, F_OK) != -1) {
          err = unlink(fapl->data_fn);
          if (0 != err) {
            ERRORMSG("Couldn't delete %s", fapl->data_fn);
          }
        }
      }
      MPI_Barrier(MPI_COMM_WORLD);
    } else if (H5F_ACC_EXCL == flags) {
      ERRORMSG("File exists %s", fapl->db_fn);
    } else {
      ERRORMSG("Unknown flag");
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);


  file->object.location = strdup(FILE_DEFAULT_PATH);
  file->object.name     = strdup(basename((char *)fname));
  file->object.root     = file;
  file->object.fapl     = fapl;

  DB_connect(fapl->db_fn, &file->db);

#ifdef MULTIFILE
  char *rfname = real_filename_create(fapl);
  file->fd     = open64(rfname, O_RDWR | O_CREAT, 0666);
  real_filename_destroy(rfname);
#elif ADAPTIVE
  // FILE CREATE
  // typedef struct SQF_t {
  //     SQO_t object;
  //     int fd;
  //     void* db;
  //     off64_t offset; // global offset
  // } SQF_t;
  //
  // file is => SQF_T* file;

  // TODO: check if this is really needed

  //char* rfname = real_filename_create(fapl);
  //file->fd = open64(rfname, O_RDWR | O_CREAT, 0666 );
  //real_filename_destroy(rfname);
  file->fd = -1;
  // this is to early for tier selection, defer until read/write
  // Option 1: Set invalid file handle?  -1   => then read/write knows it has to invoke esdm_suggest_tier()
#else
  file->fd = open64(fapl->data_fn, O_RDWR | O_CREAT, 0666);
#endif
  file->offset = 0;

  MPI_Barrier(MPI_COMM_WORLD);
  if (0 == fapl->mpi_rank) {
    //		DBF_create(file, flags, file->db, fcpl_id, fapl_id);
    DBF_create(file, flags, file->db, fcpl_id, fapl_id);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  /* Create root group */
  hid_t gcpl_id = H5Pcreate(H5P_GROUP_CREATE);
  hid_t gapl_id = H5Pcreate(H5P_GROUP_CREATE);
  hid_t gxpl_id = H5Pcreate(H5P_GROUP_CREATE);
  H5VL_loc_params_t loc_params;
  SQG_t group;
  group.object.location = create_path((SQO_t *)file);
  group.object.name     = strdup("/");
  group.object.root     = file;
  group.object.fapl     = fapl;
  SQO_init_info(&group.object.info);
  group.object.info.type = H5O_TYPE_GROUP;
  MPI_Barrier(MPI_COMM_WORLD);
  if (0 == fapl->mpi_rank) {
    DBG_create(&group, loc_params, gcpl_id, gapl_id, dxpl_id);
    //		DBG_create(&group, loc_params, gcpl_id, gapl_id, dxpl_id);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  return (void *)file;
}


static void *
H5VL_extlog_file_open(const char *fname, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  SQF_t *file;
  file = (SQF_t *)calloc(1, sizeof(SQF_t));

  h5sqlite_fapl_t *info = (h5sqlite_fapl_t *)H5VL_extlog_fapl_copy(H5Pget_vol_info(fapl_id));
  MPI_Comm_rank(MPI_COMM_WORLD, &info->mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &info->mpi_size);

  DB_connect(info->db_fn, &file->db);
#ifdef MULTIFILE
  char *rfname = real_filename_create(info);
  file->fd     = open64(rfname, O_RDWR | O_CREAT, 0666);
  real_filename_destroy(rfname);
#elif ADAPTIVE
  // FILE OPEN

  // typedef struct SQF_t {
  //     SQO_t object;
  //     int fd;
  //     void* db;
  //     off64_t offset; // global offset
  // } SQF_t;
  //
  // file is => SQF_T* file;

  // TODO: check if this is really needed
  //char* rfname = real_filename_create(info);
  //file->fd = open64(rfname, O_RDWR | O_CREAT, 0666 );
  //real_filename_destroy(rfname);
  file->fd = -1;
  // if this would only read everything is decided already, we just have rediscover which tier applies
  // this is to early for tier selection, defer until read/write
  // Option 1: Set invalid file handle?  -1   => then read/write knows it has to invoke esdm_suggest_tier()
#else
  file->fd = open64(info->data_fn, O_RDWR | O_CREAT, 0666);
#endif

  file->object.location = strdup(FILE_DEFAULT_PATH);
  file->object.name     = strdup(basename((char *)fname));
  file->object.root     = file;
  file->object.fapl     = info;
  return (void *)file;
}


static herr_t
H5VL_extlog_file_get(void *obj, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  SQF_t *file      = (SQF_t *)obj;
  herr_t ret_value = 0;
  hid_t *ret_id;

  switch (get_type) {
    case H5VL_FILE_GET_FAPL: {
      hid_t fapl_id = 0;
      DBF_get_fapl(file, &fapl_id);
      ret_id  = va_arg(arguments, hid_t *);
      *ret_id = fapl_id;
    } break;
    case H5VL_FILE_GET_FCPL: {
      hid_t fcpl_id = 0;
      DBF_get_fcpl(file, &fcpl_id);
      ret_id  = va_arg(arguments, hid_t *);
      *ret_id = fcpl_id;
    } break;
    case H5VL_FILE_GET_INTENT:
      ERRORMSG("Not implemented");
      break;
    case H5VL_FILE_GET_NAME:
      ERRORMSG("Not implemented");
      break;
    case H5VL_FILE_GET_OBJ_COUNT:
      ERRORMSG("Not implemented");
      break;
    case H5VL_FILE_GET_OBJ_IDS:
      ERRORMSG("Not implemented");
      break;
    case H5VL_OBJECT_GET_FILE:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Not supported");
  }
  return ret_value;
}


static herr_t
H5VL_extlog_file_close(void *obj, hid_t dxpl_id, void **req) {
  //	TRACEMSG("");
  SQF_t *file = (SQF_t *)obj;
  close(file->fd);
  H5VL_extlog_fapl_free(file->object.fapl);
  DB_disconnect(file->db);
  free(file->object.fapl);
  file->object.fapl = NULL;
  free(file->object.name);
  file->object.name = NULL;
  free(file);
  return 1;
}

static herr_t H5VL_extlog_file_specific(void *obj, H5VL_file_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  SQF_t *file = (SQF_t *)obj;

  switch (specific_type) {
    case H5VL_FILE_FLUSH:
      fsync(file->object.root->fd);
      break;
    case H5VL_FILE_IS_ACCESSIBLE:
      ERRORMSG("Not implemented");
      break;
    case H5VL_FILE_MOUNT:
      ERRORMSG("Not implemented");
      break;
    case H5VL_FILE_UNMOUNT:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Uknown type");
  }

  return 0;
}


static void *
H5VL_extlog_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  SQG_t *group = NULL;
  group        = (SQG_t *)malloc(sizeof(*group));

  switch (loc_params.obj_type) {
    case H5I_FILE:
    case H5I_GROUP: {
      SQO_t *sqo             = (SQO_t *)obj;
      group->object.location = create_path(sqo);
      group->object.name     = strdup(name);
      group->object.root     = sqo->root;
      group->object.fapl     = sqo->fapl;

      SQO_init_info(&group->object.info);
      group->object.info.fileno = 0;
      group->object.info.addr   = ++addr_g;
      group->object.info.num_attrs += 1;
      group->object.info.type += H5O_TYPE_GROUP;
      MPI_Barrier(MPI_COMM_WORLD);
      if (0 == group->object.fapl->mpi_rank) {
        DBG_create(group, loc_params, gcpl_id, gapl_id, dxpl_id);
      }
      MPI_Barrier(MPI_COMM_WORLD);
    } break;
    case H5I_DATATYPE:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATASET:
      ERRORMSG("Not implemented");
      break;
    case H5I_ATTR:
      ERRORMSG("Not implemented");
      break;
    case H5I_UNINIT:
    case H5I_BADID:
    case H5I_DATASPACE:
    case H5I_REFERENCE:
    case H5I_VFL:
    case H5I_VOL:
    case H5I_GENPROP_CLS:
    case H5I_GENPROP_LST:
    case H5I_ERROR_CLASS:
    case H5I_ERROR_MSG:
    case H5I_ERROR_STACK:
    case H5I_NTYPES:
      ERRORMSG("Not implemented");
    default:
      ERRORMSG("Not supported");
  } /* end switch */
  return (void *)group;
}

static void *H5VL_extlog_group_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  SQG_t *group = (SQG_t *)malloc(sizeof(*group));

  switch (loc_params.obj_type) {
    case H5I_FILE:
    case H5I_GROUP:
      MPI_Barrier(MPI_COMM_WORLD);
      DBG_open(obj, loc_params, name, group);
      break;
    case H5I_DATATYPE:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATASET:
      ERRORMSG("Not implemented");
      break;
    case H5I_ATTR:
      ERRORMSG("Not implemented");
      break;
    case H5I_UNINIT:
    case H5I_BADID:
    case H5I_DATASPACE:
    case H5I_REFERENCE:
    case H5I_VFL:
    case H5I_VOL:
    case H5I_GENPROP_CLS:
    case H5I_GENPROP_LST:
    case H5I_ERROR_CLASS:
    case H5I_ERROR_MSG:
    case H5I_ERROR_STACK:
    case H5I_NTYPES:
      ERRORMSG("Not implemented");
    default:
      ERRORMSG("Not supported");
  } /* end switch */

  return group;
}


static herr_t H5VL_extlog_group_get(void *obj, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  SQG_t *group     = (SQG_t *)obj;
  herr_t ret_value = 0;
  switch (get_type) {
    case H5VL_GROUP_GET_GCPL: {
      hid_t *gcpl_id = va_arg(arguments, hid_t *);
      DBG_get_gcpl(group, gcpl_id);
    } break;
    case H5VL_GROUP_GET_INFO:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Unsupported type");
  }
  return ret_value;
}


static herr_t
H5VL_extlog_group_close(void *grp, hid_t dxpl_id, void **req) {
  //	TRACEMSG("");
  SQG_t *g = (SQG_t *)grp;
  free(g->object.location);
  g->object.location = NULL;
  free(g->object.name);
  g->object.name = NULL;
  free(g);
  g = NULL;
  return 0;
}


static herr_t H5VL_extlog_group_specific(void *obj, H5VL_group_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  ERRORMSG("Not implemented");
  herr_t ret_value = 0;
  return ret_value;
}


static herr_t H5VL_extlog_group_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  ERRORMSG("Not implemented");
  herr_t ret_value = 0;
  return ret_value;
}


static void *
H5VL_extlog_datatype_commit(void *obj, H5VL_loc_params_t loc_params, const char *name,
hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  ERRORMSG("Not implemented");
  SQD_t *o = (SQD_t *)obj;
  return NULL;
}

static void *
H5VL_extlog_datatype_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t tapl_id, hid_t dxpl_id, void **req) {
  TRACEMSG("");
  ERRORMSG("Not implemented");
  SQD_t *o = (SQD_t *)obj;
  return NULL;
}

static herr_t
H5VL_extlog_datatype_get(void *dt, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  ERRORMSG("Not implemented");
  herr_t ret_value = 0;
  return ret_value;
}

static herr_t
H5VL_extlog_datatype_close(void *dt, hid_t dxpl_id, void **req) {
  //	TRACEMSG("");
  ERRORMSG("Not implemented");
  return 1;
}

//	static void *
//H5VL_extlog_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req)
//{
//	assert(false);
//	return NULL;
//}
//
//	static herr_t
//H5VL_extlog_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type,
//		hid_t dxpl_id, void **req, va_list arguments)
//{
//	TRACEMSG("");
//	return 1;
//}


static void *
H5VL_extlog_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req) {
  DEBUGMSG("%s", name);
  SQD_t *dset = NULL;
  dset        = (SQD_t *)malloc(sizeof(*dset));

  hid_t space_id;
  H5Pget(dcpl_id, "dataset_space_id", &space_id);
  int ndims = H5Sget_simple_extent_ndims(space_id);
  hsize_t maxdims[ndims];
  hsize_t dims[ndims];
  H5Sget_simple_extent_dims(space_id, dims, maxdims);

  hid_t type_id;
  H5Pget(dcpl_id, "dataset_type_id", &type_id);
  size_t type_size = H5Tget_size(type_id);
  size_t data_size = type_size;
  for (int i = 0; i < ndims; ++i) {
    data_size *= dims[i];
  }

  switch (loc_params.obj_type) {
    case H5I_FILE:
    case H5I_GROUP: {
      SQO_t *sqo            = (SQO_t *)obj;
      dset->object.root     = sqo->root;
      dset->object.fapl     = sqo->fapl;
      dset->object.location = create_path(sqo);
      dset->object.name     = strdup(name);
      dset->data_size       = data_size;
      dset->offset          = sqo->root->offset;
#ifdef MULTIFILE
      dset->object.root->offset += data_size / dset->object.fapl->mpi_size;
#elif ADAPTIVE
      // DATASET CREATE
      dset->object.root->offset += data_size / dset->object.fapl->mpi_size;

      // at this time the create property would be known for the write case
      // but there seems to be no benefit to already decide on a tier..
      // also the SQF_t is not availabe here yet.. would need to traverse the tree
#else
      dset->object.root->offset += data_size; // free position for the next datasets
#endif
      SQO_init_info(&dset->object.info);
      dset->object.info.fileno    = sqo->root->fd;
      dset->object.info.addr      = ++addr_g; // todo
      dset->object.info.num_attrs = 0;
      dset->object.info.type      = H5O_TYPE_DATASET;
      dset->object.info.btime     = ++counter_g; // todo

      MPI_Barrier(MPI_COMM_WORLD);
      if (0 == sqo->fapl->mpi_rank) {
        DBD_create(dset, loc_params, dcpl_id, dapl_id, dxpl_id);
      }
      MPI_Barrier(MPI_COMM_WORLD);
    } break;
    case H5I_DATATYPE:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATASET:
      ERRORMSG("Not implemented");
      break;
    case H5I_ATTR:
      ERRORMSG("Not implemented");
      break;
    case H5I_UNINIT:
    case H5I_BADID:
    case H5I_DATASPACE:
    case H5I_REFERENCE:
    case H5I_VFL:
    case H5I_VOL:
    case H5I_GENPROP_CLS:
    case H5I_GENPROP_LST:
    case H5I_ERROR_CLASS:
    case H5I_ERROR_MSG:
    case H5I_ERROR_STACK:
    case H5I_NTYPES:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Not supported");
  } /* end switch */

  return (void *)dset;
}

static void *
H5VL_extlog_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req) {
  DEBUGMSG("%s", name);
  SQD_t *dset = (SQD_t *)malloc(sizeof(*dset));

  switch (loc_params.obj_type) {
    case H5I_FILE:
    case H5I_GROUP: {
      DBD_open(obj, loc_params, name, dset);
    } break;
    case H5I_DATATYPE:
      ERRORMSG("Not implemented");
      break;
    case H5I_DATASET:
      ERRORMSG("Not implemented");
      break;
    case H5I_ATTR:
      ERRORMSG("Not implemented");
      break;
    case H5I_UNINIT:
    case H5I_BADID:
    case H5I_DATASPACE:
    case H5I_REFERENCE:
    case H5I_VFL:
    case H5I_VOL:
    case H5I_GENPROP_CLS:
    case H5I_GENPROP_LST:
    case H5I_ERROR_CLASS:
    case H5I_ERROR_MSG:
    case H5I_ERROR_STACK:
    case H5I_NTYPES:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Not supported");
  } /* end switch */

  return dset;
}


static herr_t
H5VL_extlog_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  SQD_t *d         = (SQD_t *)dset;
  herr_t ret_value = 0;
  //	hid_t type_id = 0;
  //	hid_t space_id = 0;
  hid_t *ret_id;
  //	MPI_Barrier(MPI_COMM_WORLD);
  //	DBD_get(d->object.location, &type_id, &space_id, d->object.root->db);
  //	MPI_Barrier(MPI_COMM_WORLD);
  switch (get_type) {
    case H5VL_DATASET_GET_DAPL: {
      hid_t *ret_id = va_arg(arguments, hid_t *);
      DBD_get_dapl(dset, ret_id);
    } break;
    case H5VL_DATASET_GET_DCPL: {
      hid_t *ret_id = va_arg(arguments, hid_t *);
      DBD_get_dcpl(dset, ret_id);
    } break;
    case H5VL_DATASET_GET_OFFSET:
      ERRORMSG("Not implemented");
      break;
    case H5VL_DATASET_GET_SPACE: {
      hid_t *ret_id = va_arg(arguments, hid_t *);
      DBD_get_space(dset, ret_id);
    } break;
    case H5VL_DATASET_GET_SPACE_STATUS:
      ERRORMSG("Not implemented");
      break;
    case H5VL_DATASET_GET_STORAGE_SIZE:
      ERRORMSG("Not implemented");
      break;
    case H5VL_DATASET_GET_TYPE: {
      hid_t *ret_id = va_arg(arguments, hid_t *);
      DBD_get_type(dset, ret_id);
    } break;
    default:
      ERRORMSG("Not supported");
  }

  //ret_value = H5VLattr_get(o->under_object, native_plugin_id, get_type, dxpl_id, req, arguments);
  return ret_value;
}


//	return  (bend[1]*bend[2]*bend[3]*start[0] + bend[2]*bend[3]*start[1] + bend[3]*start[2] + start[3]) * type_size;
static off64_t coord_to_offset(const hsize_t *bend, const hsize_t *start, const size_t type_size, const size_t size) {
  off64_t offset = 0;
  for (size_t i = 0; i < size; ++i) {
    off64_t temp = start[i];
    for (size_t j = i + 1; j < size; ++j) {
      temp *= bend[j];
    }
    offset += temp;
  }
  return offset * type_size;
}


static ssize_t SQD_read64(SQO_t *sqo, void *buf, size_t count, off64_t offset) {
  ssize_t bytes_read_total = 0;
  ssize_t bytes_read       = 0;
  ssize_t block_size       = count;
  assert(0 <= sqo->root->fd);

  while (bytes_read_total != block_size) {
    size_t bytes_left = block_size - bytes_read_total;
    count             = (COUNT_MAX < bytes_left) ? COUNT_MAX : bytes_left;
    DEBUGMSG("in %s read %zu bytes at offset %zu", sqo->name, count, offset + bytes_read_total);
    bytes_read = pread64(sqo->root->fd, buf + bytes_read_total, count, offset + bytes_read_total);
    if (bytes_read != count) {
      ERRORMSG("bytes_read %zu count %zu errno %s", bytes_read, count, strerror(errno));
    }
    bytes_read_total += bytes_read;
  }
  return bytes_read_total;
}


static ssize_t SQD_pwrite64(SQO_t *sqo, const void *buf, size_t count, off64_t offset) {
  ssize_t bytes_written_total = 0;
  ssize_t bytes_written       = 0;
  ssize_t block_size          = count;
  assert(0 <= sqo->root->fd);

  while (bytes_written_total != block_size) {
    size_t bytes_left = block_size - bytes_written_total;
    count             = (COUNT_MAX < bytes_left) ? COUNT_MAX : bytes_left;
    DEBUGMSG("in %s write %zu bytes at offset %zu", sqo->name, count, offset + bytes_written_total);
    bytes_written = pwrite64(sqo->root->fd, ((char *)buf) + bytes_written_total, count, offset + bytes_written_total);
    if (bytes_written != count) {
      ERRORMSG("bytes_written %zu count %zu errno %s", bytes_written, count, strerror(errno));
    }
    bytes_written_total += bytes_written;
  }
  return bytes_written_total;
}


static herr_t
H5VL_extlog_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id,
hid_t file_space_id, hid_t plist_id, void *buf, void **req) {
  TRACEMSG("");
  herr_t ret = 0;
  SQD_t *d   = (SQD_t *)dset;

  int ndims = H5Sget_simple_extent_ndims(mem_space_id);
  hsize_t dims[ndims];
  hsize_t max_dims[ndims];
  H5Sget_simple_extent_dims(mem_space_id, dims, max_dims);

  size_t block_size = H5Tget_size(mem_type_id);
  assert(block_size != 0);
  for (size_t i = 0; i < ndims; ++i) {
    block_size *= dims[i];
  }

  hsize_t start[ndims];
  hsize_t stride[ndims];
  hsize_t count[ndims];
  hsize_t block[ndims];
  if (-1 == (ret = H5Sget_regular_hyperslab(file_space_id, start, stride, count, block))) {
    ERRORMSG("Couldn't read hyperslab.");
  }

  hid_t dspace_id;
  DBD_get_space(dset, &dspace_id);
  hsize_t ddims[ndims];
  hsize_t dmaxdims[ndims];
  if (H5Sis_simple(dspace_id)) {
    H5Sget_simple_extent_dims(dspace_id, ddims, dmaxdims);
  } else {
    ERRORMSG("Not supported space.");
  }

#ifdef MULTIFILE
  off64_t rel_offset = start[0] * block_size * d->object.fapl->mpi_size;
#elif ADAPTIVE
  // DATASET READ

  // block_size is:   sizeof(datatype) * dim[0] * ... * dim[n]   -> here cuboid  x*y*z
  // call to ESDM Decision Component

  // Make tier decision if not already set.
  // TODO: alternative?: d->object.root->fd
  SQO_t *esdm_sqo = (SQO_t *)dset;
  if (esdm_sqo->root->fd == -1) {
    char *esdm_tiername = esdm_suggest_tier(d->object.fapl, d->object.fapl->mpi_size, block_size);
    esdm_sqo->root->fd  = open64(esdm_tiername, O_RDWR | O_CREAT, 0666);
    real_filename_destroy(esdm_tiername);
  }

  off64_t rel_offset = start[0] * block_size * d->object.fapl->mpi_size;
#else
  off64_t rel_offset = start[0] * block_size * d->object.fapl->mpi_size + block_size * d->object.fapl->mpi_rank;
#endif

  off64_t offset = d->offset + rel_offset;

  SQD_read64(dset, buf, block_size, offset);

  return ret;
}


static herr_t
H5VL_extlog_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id,
hid_t file_space_id, hid_t plist_id, const void *buf, void **req) {
  TRACEMSG("");
  herr_t ret = 0;
  SQD_t *d   = (SQD_t *)dset;

  int ndims = H5Sget_simple_extent_ndims(mem_space_id);
  hsize_t dims[ndims];
  hsize_t max_dims[ndims];
  H5Sget_simple_extent_dims(mem_space_id, dims, max_dims);

  size_t block_size = H5Tget_size(mem_type_id);
  assert(block_size != 0);
  for (size_t i = 0; i < ndims; ++i) {
    block_size *= dims[i];
  }

  hsize_t start[ndims];
  hsize_t stride[ndims];
  hsize_t count[ndims];
  hsize_t block[ndims];
  if (-1 == (ret = H5Sget_regular_hyperslab(file_space_id, start, stride, count, block))) {
    ERRORMSG("Couldn't read hyperslab.");
  }

  hid_t dspace_id;
  DBD_get_space(dset, &dspace_id);
  hsize_t ddims[ndims];
  hsize_t dmaxdims[ndims];
  if (H5Sis_simple(dspace_id)) {
    H5Sget_simple_extent_dims(dspace_id, ddims, dmaxdims);
  } else {
    ERRORMSG("Not supported space.");
  }

#ifdef MULTIFILE
  off64_t rel_offset = start[0] * block_size * d->object.fapl->mpi_size;
#elif ADAPTIVE
  // DATASET WRITE

  // block_size is:   sizeof(datatype) * dim[0] * ... * dim[n]   -> here cuboid  x*y*z
  // call to ESDM Decision Component

  // Make tier decision if not already set.
  // TODO: alternative?: d->object.root->fd
  SQO_t *esdm_sqo = (SQO_t *)dset;
  if (esdm_sqo->root->fd == -1) {
    char *esdm_tiername = esdm_suggest_tier(d->object.fapl, d->object.fapl->mpi_size, block_size);
    esdm_sqo->root->fd  = open64(esdm_tiername, O_RDWR | O_CREAT, 0666);
    real_filename_destroy(esdm_tiername);
  }

  off64_t rel_offset = start[0] * block_size * d->object.fapl->mpi_size;
#else
  off64_t rel_offset = start[0] * block_size * d->object.fapl->mpi_size + block_size * d->object.fapl->mpi_rank;
#endif

  off64_t offset = d->offset + rel_offset;

  SQD_pwrite64(dset, buf, block_size, offset);

  fsync(d->object.root->fd);
  fdatasync(d->object.root->fd);
  return ret;
}


static herr_t
H5VL_extlog_dataset_close(void *dset, hid_t dxpl_id, void **req) {
  //	TRACEMSG("");
  SQD_t *d = (SQD_t *)dset;
  free(d->object.location);
  d->object.location = NULL;
  free(d->object.name);
  d->object.name = NULL;
  free(d);
  d = NULL;
  return 0;
}


static herr_t
H5VL_extlog_link_create(H5VL_link_create_type_t create_type, void *obj, H5VL_loc_params_t loc_params, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id, void **req) {
  ERRORMSG("Not implemented");
  return 0;
}


static herr_t
H5VL_extlog_link_copy(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req) {
  ERRORMSG("Not implemented");
  return 0;
}


static herr_t
H5VL_extlog_link_move(void *src_obj, H5VL_loc_params_t loc_params1, void *dst_obj, H5VL_loc_params_t loc_params2, hid_t lcpl, hid_t lapl, hid_t dxpl_id, void **req) {
  ERRORMSG("Not implemented");
  return 0;
}


static herr_t
H5VL_extlog_link_get(void *obj, H5VL_loc_params_t loc_params, H5VL_link_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  ERRORMSG("Not implemented");
  return 0;
}


static herr_t
H5VL_extlog_link_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_link_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) {
  SQG_t *sqg = (SQG_t *)malloc(sizeof(*sqg));
  memcpy(sqg, obj, sizeof(*sqg));
  SQO_t *obj2 = (SQO_t *)sqg;

  SQO_t *obj1    = (SQO_t *)obj;
  obj2->root     = obj1->root;
  obj2->fapl     = obj1->fapl;
  obj2->info     = obj1->info;
  obj2->location = strdup(obj1->location);
  obj2->name     = strdup(obj1->name);

  hid_t vol_id = H5VLget_plugin_id("extlog");
  assert(-1 != vol_id);
  hid_t group_id = H5VLobject_register(obj2, loc_params.obj_type, vol_id);
  assert(-1 != group_id);

  switch (specific_type) {
    case H5VL_LINK_DELETE:
      ERRORMSG("Not implemented");
      break;
    case H5VL_LINK_EXISTS:
      ERRORMSG("Not implemented");
      break;
    case H5VL_LINK_ITER: {
      hbool_t recursive     = va_arg(arguments, int);
      H5_index_t idx_type   = va_arg(arguments, H5_index_t);
      H5_iter_order_t order = va_arg(arguments, H5_iter_order_t);
      hsize_t *idx_p        = va_arg(arguments, hsize_t *);
      H5L_iterate_t op      = va_arg(arguments, H5L_iterate_t);
      void *op_data         = va_arg(arguments, void *);

      switch (idx_type) {
        case H5_INDEX_NAME:
          switch (order) {
            case H5_ITER_INC: {
              H5L_info_t *link_info   = (H5L_info_t *)malloc(sizeof(*link_info));
              link_info->type         = H5L_TYPE_HARD;
              link_info->corder_valid = false;
              link_info->corder       = 0;
              link_info->cset         = H5T_CSET_ASCII;
              //									link_info->u.val_size = 0;
              link_info->u.address = 0;

              char **dset_list      = NULL;
              size_t dset_list_size = 0;
              DB_create_name_list(obj, loc_params, "DATASETS", &dset_list, &dset_list_size);
              //									DB_create_name_list(obj, loc_params, "GROUPS", &dset_list, &dset_list_size);

              for (size_t i = 0; i < dset_list_size; ++i) {
                //										DBA_get_link_info(obj, dset_list[i], &link_info);
                op(group_id, dset_list[i], link_info, op_data);
              }

              DB_destroy_name_list(dset_list, dset_list_size);
              TODOMSG("Bullshit implementation of - H5VL_LINK_ITER / H5_INDEX_NAME / H5_ITER_INC");
            } break;
            case H5_ITER_DEC:
              ERRORMSG("Not implemented");
              break;
            case H5_ITER_NATIVE:
              ERRORMSG("Not implemented");
              break;
            default:
              ERRORMSG("Not implemented");
          }
          break;
        case H5_INDEX_CRT_ORDER:
          ERRORMSG("Not implemented");
          break;
        default:
          ERRORMSG("Not implemented");
      }

    } break;
    default:
      ERRORMSG("Not supported");
  }
  return 0;
}


static herr_t
H5VL_extlog_link_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments) {
  ERRORMSG("Not implemented");
  return 0;
}


/* H5O routines */
static void *
H5VL_extlog_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req) {
  void *ret_obj = NULL;
  switch (loc_params.type) {
    case H5VL_OBJECT_BY_NAME:
      switch (loc_params.obj_type) {
        case H5I_GROUP: {
          int entry_exists = false;
          DB_entry_exists(obj, "DATASETS", loc_params.loc_data.loc_by_name.name, &entry_exists);
          if (entry_exists) {
            ret_obj = (SQD_t *)malloc(sizeof(SQD_t));
            DBD_open(obj, loc_params, loc_params.loc_data.loc_by_name.name, ret_obj);
            *opened_type = H5I_DATASET;
          } else {
            ERRORMSG("Not implemented");
          }
        } break;
        default:
          ERRORMSG("Not implemented");
      }
      break;
    default:
      ERRORMSG("Not implemented");
  }
  assert(NULL != ret_obj);
  return ret_obj;
}


static herr_t
H5VL_extlog_object_copy(void *src_obj, H5VL_loc_params_t loc_params1, const char *src_name, void *dst_obj, H5VL_loc_params_t loc_params2, const char *dst_name, hid_t ocpypl_id, hid_t lcpl_id, hid_t dxpl_id, void **req) {
  ERRORMSG("Not implemented");
  return 0;
}

static herr_t
H5VL_extlog_object_get(void *obj, H5VL_loc_params_t loc_params, H5VL_object_get_t get_type, hid_t dxpl_id, void **req, va_list arguments) {
  switch (get_type) {
    case H5VL_REF_GET_NAME:
      ERRORMSG("Not implemented");
      break;
    case H5VL_REF_GET_REGION:
      ERRORMSG("Not implemented");
      break;
    case H5VL_REF_GET_TYPE:
      ERRORMSG("Not implemented");
      break;
    default:
      ERRORMSG("Unknown type");
  }
  return 0;
}


static herr_t H5VL_extlog_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments) {
  switch (specific_type) {
    case H5VL_OBJECT_CHANGE_REF_COUNT:
      ERRORMSG("Not implemented");
      break;
    case H5VL_OBJECT_EXISTS:
      ERRORMSG("Not implemented");
      break;
    case H5VL_OBJECT_VISIT:
      ERRORMSG("Not implemented");
      break;
    case H5VL_REF_CREATE: {
      void *ref           = va_arg(arguments, void *);
      const char *name    = va_arg(arguments, char *);
      H5R_type_t ref_type = va_arg(arguments, H5R_type_t);
      hid_t space_id      = va_arg(arguments, hid_t);
      //				H5S_t       *space = NULL;   /* Pointer to dataspace containing region */

      TODOMSG("Not implemented H5VL_REF_CREATE");
    } break;
    default:
      ERRORMSG("Unknown type");
  }
  return 0;
}


static herr_t
H5VL_extlog_object_optional(void *obj, hid_t dxpl_id, void **req, va_list arguments) {
  TRACEMSG("");
  H5VL_object_optional_t optional_type = va_arg(arguments, H5VL_object_optional_t);
  H5VL_loc_params_t loc_params         = va_arg(arguments, H5VL_loc_params_t);

  //    H5G_loc_t	loc;                    /* Location of group */
  herr_t ret_value = 0; /* Return value */

  switch (optional_type) {
    /* H5Oget_info / H5Oget_info_by_name / H5Oget_info_by_idx */
    case H5VL_OBJECT_GET_INFO: {
      H5O_info_t *obj_info = va_arg(arguments, H5O_info_t *);
      SQO_t *sqo           = (SQO_t *)obj;

      if (loc_params.type == H5VL_OBJECT_BY_SELF) { /* H5Oget_info */
        *obj_info = sqo->info;
      } else if (loc_params.type == H5VL_OBJECT_BY_NAME) { /* H5Oget_info_by_name */
        ERRORMSG("Not implemented");
      } else if (loc_params.type == H5VL_OBJECT_BY_IDX) { /* H5Oget_info_by_idx */
        ERRORMSG("Not implemented");
      } else {
        ERRORMSG("Not supported");
      }
      break;
    }
      /* H5Oget_comment / H5Oget_comment_by_name */
    case H5VL_OBJECT_GET_COMMENT: {
      char *comment  = va_arg(arguments, char *);
      size_t bufsize = va_arg(arguments, size_t);
      ssize_t *ret   = va_arg(arguments, ssize_t *);

      /* Retrieve the object's comment */
      if (loc_params.type == H5VL_OBJECT_BY_SELF) { /* H5Oget_comment */
        ERRORMSG("Not implemented");
      } else if (loc_params.type == H5VL_OBJECT_BY_NAME) { /* H5Oget_comment_by_name */
        ERRORMSG("Not implemented");
      } else {
        ERRORMSG("Not supported");
      }
      break;
    }
      /* H5Oset_comment */
    case H5VL_OBJECT_SET_COMMENT: {
      const char *comment = va_arg(arguments, char *);

      if (loc_params.type == H5VL_OBJECT_BY_SELF) { /* H5Oset_comment */
        ERRORMSG("Not implemented");
      } else if (loc_params.type == H5VL_OBJECT_BY_NAME) { /* H5Oset_comment_by_name */
        ERRORMSG("Not implemented");
      } else {
        ERRORMSG("Not supported");
      }
      break;
    }
    default:
      ERRORMSG("Not supported");
  }
  return 0;
}


/* return the library type which should always be H5PL_TYPE_VOL */
H5PL_type_t
H5PLget_plugin_type(void) {
  TRACEMSG("");
  return H5PL_TYPE_VOL;
}
/* return a pointer to the plugin structure defining the VOL plugin with all callbacks */
const void *H5PLget_plugin_info(void) {
  TRACEMSG("");
  return &H5VL_log_g;
}
