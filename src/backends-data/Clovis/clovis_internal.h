/* This file is part of ESDM.                                              
 *                                                                              
 * This program is is free software: you can redistribute it and/or modify         
 * it under the terms of the GNU Lesser General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or            
 * (at your option) any later version.                                          
 *                                                                              
 * This program is is distributed in the hope that it will be useful,           
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                
 * GNU General Public License for more details.                                 
 *                                                                                 
 * You should have received a copy of the GNU Lesser General Public License        
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.           
 *
 * Original author: Huang Hua <hua.huang@seagate.com>
 * Original creation date: 13-Oct-2017
 */   
#ifndef CLOVIS_INTERNAL_H
#define CLOVIS_INTERNAL_H

#include "clovis/clovis.h"
#include "clovis/clovis_idx.h"
#include "lib/memory.h"

struct esdm_backend_xxxops {
	/**
	 * Allocate the backend context and initialise it.
	 * In this interface, Clovis connects to Mero cluster.
	 * @param [in] conf, the connection parameters.
	 * @param [out] eb_out, returned the backend context.
	 *
	 * The format of conf string would like this:
	 * "laddr ha_addr prof_opt proc_fid".
	 */
	int (*esdm_backend_init)(char * conf, esdm_backend_t *eb);

	/**
	 * Finalise the backend.
	 */
	int (*esdm_backend_fini)(esdm_backend_t *eb);

	/* object operations start here */
	/**
	 * Allocate an object or objects in backend and return them
	 * in @out out_object_id and their internal attributes @out out_mero_metadata.
	 * @param [in] eb, the backend pointer.
	 * @param [in] n_dims, number of dimensions.
	 * @param [in] dims_size, dimension array.
	 * @param [in] type, type of esdm.
	 * @param [in] md1, metadata.
	 * @param [in] md2, metadata.
	 * @param [out] out_object_id, the returned objects.
	 * @param [out] out_mero_metadata, the returned metadata.
	 */
	int (*esdm_backend_obj_alloc)(esdm_backend_t *eb,
				      int       n_dims,
				      int*      dims_size,
				      esdm_type type,
				      char*     md1,
				      char*     md2,
				      char**    out_object_id,
				      char**    out_mero_metadata);

	/**
	 * Open an object for read/write.
	 *
	 * return object handle in [out] obj_handle;
	 */
	int (*esdm_backend_obj_open) (esdm_backend_t *eb,
				      char*    object_id,
				      void**   obj_handle);

	/**
	 * Write to an object.
	 */
	int (*esdm_backend_obj_write)(esdm_backend_t *eb,
				      void*    obj_handle,
				      uint64_t start,
				      uint64_t count,
				      void*    data);

	/**
	 * Read from object.
	 */
	int (*esdm_backend_obj_read) (esdm_backend_t *eb,
				      void*    obj_handle,
				      uint64_t start,
				      uint64_t count,
				      void*    data);

	/**
	 * Close an object.
	 */
	int (*esdm_backend_obj_close)(esdm_backend_t *eb,
				      void*  obj_handle);

    /**
     * Insert an mapping (fragment name -> object id) into internal index.
     */
    int (*mapping_insert)(esdm_backend_t  *backend,
                          const char *name,
                          const char *obj_id);
    /**
     * Query an mapping (fragment name -> object id) into internal index.
     */
    int (*mapping_get)   (esdm_backend_t  *backend,
                          const char *name,
                          char      **obj_id);
};

typedef struct {
	esdm_backend_t              ebm_base;

	/* Mero Clovis */
	struct m0_clovis*           ebm_clovis_instance;
	struct m0_clovis_container  ebm_clovis_container;
	struct m0_clovis_config     ebm_clovis_conf;

	struct m0_fid               ebm_last_fid;

    /* for test */
    struct esdm_backend_xxxops  ebm_ops;
} esdm_backend_clovis_t;


extern esdm_backend_clovis_t esdm_backend_clovis;

#endif
