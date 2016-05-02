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

static GHashTable * files_tbl = NULL;

static void * memvol_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    if(files_tbl == NULL){
      files_tbl = g_hash_table_new (g_str_hash,g_str_equal);
    }

    memvol_file_t *file;

    file = g_hash_table_lookup (files_tbl, name);

    if((flags & H5F_ACC_EXCL) && file != NULL){
      // invalid https://www.hdfgroup.org/HDF5/doc/RM/RM_H5F.html#File-Create
      return NULL;
    }

    if((flags & H5F_ACC_TRUNC) && file != NULL){
      // TODO: truncate the file. Free all structures...
      memvol_group_init(& file->root_grp);
    }

    if ( file == NULL ){
      file = (memvol_file_t *) malloc(sizeof(memvol_file_t));
      memvol_group_init(& file->root_grp);
      file->name = strdup(name);
      file->fcpl_id = H5Pcopy(fcpl_id);
    }
    if( flags & H5F_ACC_RDONLY){
      file->mode_flags = H5F_ACC_RDONLY;
    }else if (flags & H5F_ACC_RDWR){
      file->mode_flags = H5F_ACC_RDWR;
    }else if (flags & H5F_ACC_TRUNC){
      file->mode_flags = H5F_ACC_RDWR;
    }else{
      assert(0 && "Modeflags are invalid");
    }
    file->mode_flags = flags;
    file->fapl_id = H5Pcopy(fapl_id);

    debugI("Fcreate %p %s\n", (void*) file, name);

    return (void *)file;
}

static void * memvol_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    memvol_file_t *file;
    file = g_hash_table_lookup (files_tbl, name);

    return (void *)file;
}

static herr_t memvol_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_file_t *f = (memvol_file_t *)file;

    switch (get_type) {
      /* H5Fget_access_plist */
      case H5VL_FILE_GET_FAPL:
      {
        hid_t *plist_id = va_arg (arguments, hid_t *);
        *plist_id = H5Pcopy(f->fapl_id);

        break;
      }
      /* H5Fget_create_plist */
      case H5VL_FILE_GET_FCPL:
      {
        hid_t *plist_id = va_arg (arguments, hid_t *);
        *plist_id = H5Pcopy(f->fcpl_id);
        break;
      }
      /* H5Fget_obj_count */
      case H5VL_FILE_GET_OBJ_COUNT:
      {
        unsigned types = va_arg (arguments, unsigned);
        ssize_t *ret = va_arg (arguments, ssize_t *);
        size_t  obj_count = 0;      /* Number of opened objects */
        assert(0 && "TODO");
        /* Set the return value */
        *ret = (ssize_t)obj_count;
        break;
      }
      /* H5Fget_obj_ids */
      case H5VL_FILE_GET_OBJ_IDS:
      {
        unsigned types = va_arg (arguments, unsigned);
        size_t max_objs = va_arg (arguments, size_t);
        hid_t *oid_list = va_arg (arguments, hid_t *);
        ssize_t *ret = va_arg (arguments, ssize_t *);
        size_t  obj_count = 0;      /* Number of opened objects */

        assert(0 && "TODO");

        /* Set the return value */
        *ret = (ssize_t)obj_count;
        break;
      }
      /* H5Fget_intent */
      case H5VL_FILE_GET_INTENT:
      {
        unsigned *ret = va_arg (arguments, unsigned *);
        *ret = f->mode_flags;
        break;
      }
      /* H5Fget_name */
      case H5VL_FILE_GET_NAME:
      {
        H5I_type_t type = va_arg (arguments, H5I_type_t);
        size_t     size = va_arg (arguments, size_t);
        char      *name = va_arg (arguments, char *);
        ssize_t   *ret  = va_arg (arguments, ssize_t *);
        size_t     len = strlen(f->name);

        if(name) {
          strncpy(name, f->name, MIN(len + 1,size));
          if(len >= size) name[size-1]='\0';
        }

        /* Set the return value for the API call */
        *ret = (ssize_t)len;
        break;
      }
      /* H5I_get_file_id */
      case H5VL_OBJECT_GET_FILE:
      {
        H5I_type_t type = va_arg (arguments, H5I_type_t);
        void ** ret = va_arg (arguments, void **);
        void * tmp;
        assert(0 && "TODO");

        switch(type) {
          case H5I_FILE:
          tmp = f;
          break;
          case H5I_GROUP:
            break;
          case H5I_DATATYPE:
            break;
          case H5I_DATASET:
            break;
          case H5I_ATTR:
            break;
          default:
            assert(0 && "Invalid datatype");
        }

        *ret = (void*) tmp;
        break;
      }
      default:
        assert(0);
    } /* end switch */

    return 1;
}

static herr_t memvol_file_close(void *file, hid_t dxpl_id, void **req)
{
    return 0;
}
