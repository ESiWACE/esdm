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



// extract from ../install/download/vol/src/H5Fpkg.h:233 for reference (consider any structure strictly private!)
/*
 * Define the structure to store the file information for HDF5 files. One of
 * these structures is allocated per file, not per H5Fopen(). That is, set of
 * H5F_t structs can all point to the same H5F_file_t struct. The `nrefs'
 * count in this struct indicates the number of H5F_t structs which are
 * pointing to this struct.
 */
//struct H5F_file_t {
//    H5FD_t	*lf; 		/* Lower level file handle for I/O	*/
//    H5F_super_t *sblock;        /* Pointer to (pinned) superblock for file */
//    H5O_drvinfo_t *drvinfo;	/* Pointer to the (pinned) driver info
//                                 * cache entry.  This field is only defined
//                                 * for older versions of the super block,
//                                 * and then only when a driver information
//                                 * block is present.  At all other times
//                                 * it should be NULL.
//                                 */
//    unsigned	nrefs;		/* Ref count for times file is opened	*/
//    unsigned	flags;		/* Access Permissions for file          */
//    H5F_mtab_t	mtab;		/* File mount table                     */
//    H5F_efc_t   *efc;           /* External file cache                  */
//
//    /* Cached values from FCPL/superblock */
//    uint8_t	sizeof_addr;	/* Size of addresses in file            */
//    uint8_t	sizeof_size;	/* Size of offsets in file              */
//    haddr_t	sohm_addr;	/* Relative address of shared object header message table */
//    unsigned	sohm_vers;	/* Version of shared message table on disk */
//    unsigned	sohm_nindexes;	/* Number of shared messages indexes in the table */
//    unsigned long feature_flags; /* VFL Driver feature Flags            */
//    haddr_t	maxaddr;	/* Maximum address for file             */
//
//    H5AC_t      *cache;		/* The object cache	 		*/
//    H5AC_cache_config_t
//		mdc_initCacheCfg; /* initial configuration for the      */
//                                /* metadata cache.  This structure is   */
//                                /* fixed at creation time and should    */
//                                /* not change thereafter.               */
//    hid_t       fcpl_id;	/* File creation property list ID 	*/
//    H5F_close_degree_t fc_degree;   /* File close behavior degree	*/
//    size_t	rdcc_nslots;	/* Size of raw data chunk cache (slots)	*/
//    size_t	rdcc_nbytes;	/* Size of raw data chunk cache	(bytes)	*/
//    double	rdcc_w0;	/* Preempt read chunks first? [0.0..1.0]*/
//    size_t      sieve_buf_size; /* Size of the data sieve buffer allocated (in bytes) */
//    hsize_t	threshold;	/* Threshold for alignment		*/
//    hsize_t	alignment;	/* Alignment				*/
//    unsigned	gc_ref;		/* Garbage-collect references?		*/
//    unsigned	latest_flags;	/* The latest version support */
//    hbool_t	store_msg_crt_idx;  /* Store creation index for object header messages?	*/
//    unsigned	ncwfs;		/* Num entries on cwfs list		*/
//    struct H5HG_heap_t **cwfs;	/* Global heap cache			*/
//    struct H5G_t *root_grp;	/* Open root group			*/
//    H5FO_t *open_objs;          /* Open objects in file                 */
//    H5UC_t *grp_btree_shared;   /* Ref-counted group B-tree node info   */
//
//    /* File space allocation information */
//    H5F_file_space_type_t fs_strategy;	/* File space handling strategy		*/
//    hsize_t     fs_threshold;	/* Free space section threshold 	*/
//    hbool_t     use_tmp_space;  /* Whether temp. file space allocation is allowed */
//    haddr_t	tmp_addr;       /* Next address to use for temp. space in the file */
//    unsigned fs_aggr_merge[H5FD_MEM_NTYPES];    /* Flags for whether free space can merge with aggregator(s) */
//    H5F_fs_state_t fs_state[H5FD_MEM_NTYPES];   /* State of free space manager for each type */
//    haddr_t fs_addr[H5FD_MEM_NTYPES];   /* Address of free space manager info for each type */
//    H5FS_t *fs_man[H5FD_MEM_NTYPES];    /* Free space manager for each file space type */
//    H5FD_mem_t fs_type_map[H5FD_MEM_NTYPES]; /* Mapping of "real" file space type into tracked type */
//    H5F_blk_aggr_t meta_aggr;   /* Metadata aggregation info */
//                                /* (if aggregating metadata allocations) */
//    H5F_blk_aggr_t sdata_aggr;  /* "Small data" aggregation info */
//                                /* (if aggregating "small data" allocations) */
//
//    /* Metadata accumulator information */
//    H5F_meta_accum_t accum;     /* Metadata accumulator info           	*/
//};
//
/*
 * This is the top-level file descriptor.  One of these structures is
 * allocated every time H5Fopen() is called although they may contain pointers
 * to shared H5F_file_t structs.
 */
//struct H5F_t {
//    char		*open_name;	/* Name used to open file	*/
//    char		*actual_name;	/* Actual name of the file, after resolving symlinks, etc. */
//    char               	*extpath;       /* Path for searching target external link file */
//    H5F_file_t		*shared;	/* The shared file info		*/
//    unsigned		nopen_objs;	/* Number of open object headers*/
//    H5FO_t              *obj_count;     /* # of time each object is opened through top file structure */
//    hbool_t             id_exists;      /* Whether an ID for this struct exists   */
//    hbool_t             closing;        /* File is in the process of being closed */
//    struct H5F_t        *parent;        /* Parent file that this file is mounted to */
//    unsigned            nmounts;        /* Number of children mounted to this file */
//#ifdef H5_HAVE_PARALLEL
//    H5P_coll_md_read_flag_t coll_md_read;  /* Do all metadata reads collectively */
//    hbool_t             coll_md_write;  /* Do all metadata writes collectively */
//#endif /* H5_HAVE_PARALLEL */
//};



// ../install/download/vol/src/H5VLnative.c
// ../install/download/vol/src/H5G.c



static GHashTable * files_tbl = NULL;

static void * memvol_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    memvol_object_t *object;
    memvol_file_t *file;

	debugI("%s\n", __func__);


	// create files hash map if not already existent
	if(files_tbl == NULL){
		files_tbl = g_hash_table_new (g_str_hash,g_str_equal);
	}

	// lookup the filename in the lsit of files
	file = g_hash_table_lookup (files_tbl, name);


	debugI("%s: files_tbl.size=%d\n", __func__, g_hash_table_size(files_tbl));


	// conform to HDF5: invalid https://www.hdfgroup.org/HDF5/doc/RM/RM_H5F.html#File-Create
	if((flags & H5F_ACC_EXCL) && file != NULL){
		return NULL;
	}

	if((flags & H5F_ACC_TRUNC) && file != NULL){
		// TODO: truncate the file. Free all structures...
		memvol_group_init(& file->root_grp);
	}


	// create the file if not already existent
	if ( file == NULL ){

		// allocate resources
		object = (memvol_object_t*) malloc(sizeof(memvol_object_t));
		file = (memvol_file_t *) malloc(sizeof(memvol_file_t));

		// populate file and object data strutures
		memvol_group_init(& file->root_grp);

		object->type = MEMVOL_GROUP;
		object->object =  & file->root_grp;

		g_hash_table_insert( file->root_grp.childs_tbl, strdup("/"), object);
		g_hash_table_insert( files_tbl, strdup(name), object);

		file->name = strdup(name);
		file->fcpl_id = H5Pcopy(fcpl_id);
	}

	// validate and set flags
	if( flags & H5F_ACC_RDONLY) {
		file->mode_flags = H5F_ACC_RDONLY;

	} else if (flags & H5F_ACC_RDWR) {
		file->mode_flags = H5F_ACC_RDWR;

	} else if (flags & H5F_ACC_TRUNC) {
		file->mode_flags = H5F_ACC_RDWR;

	} else {
		assert(0 && "Modeflags are invalid");
	}

	// attach to file struct
    file->mode_flags = flags;
    file->fapl_id = H5Pcopy(fapl_id);

    debugI("%s: New file=%p with name=%s\n", __func__, (void*) file, name);


	debugI("%s: files_tbl.size=%d\n", __func__, g_hash_table_size(files_tbl));



    return (void *)object;
}

static void * memvol_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    memvol_object_t *object;

	debugI("%s\n", __func__);

    object = g_hash_table_lookup (files_tbl, name);

    return (void *)object;
}

static herr_t memvol_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_file_t *f = (memvol_file_t *)file;

	debugI("%s\n", __func__);

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
	debugI("%s\n", __func__);

    return 0;
}
