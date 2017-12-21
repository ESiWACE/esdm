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
 */   
#ifndef WOS_H
#define WOS_H

// Internal functions used by this backend.

typedef struct {
        //struct esdm_backend_generic ebm_base;

        /* WOS */
	t_WosClusterPtr*	      wos_cluster;	//Pointer to WOS cluster
	t_WosPolicy* 		      wos_policy;	//Policy
	t_WosObjPtr* 		      wos_meta_obj;	//Used for internal purposes to store metadata info
	t_WosOID** 		      oid_list;		//List of objects ids

} esdm_backend_wos;

/**
 * Open a connection to the WOS cluster
 * return a handle in wos_handle
 *
 * @param [out] wos_handle		The pointer to a wos specific handle
 *
 * @return Status.
 */
esdm_status_t esdm_backend_wos_open (void*   wos_handle);


/**
 * Allocate a pool of objects in wos backend and return the related oids
 * in @out out_object_id and their internal attributes in @out out_wos_metadata.
 *
 * @param [in] n_dims 			number of dimensions.
 * @param [in] dims_size		array of dimensions' size.
 * @param [in] type			type of data to be stored.
 * @param [in] md1			metadata.
 * @param [in] md2			metadata.
 * @param [in] w_handle			Pointer to a wos specific handle.
 * @param [out] out_object_id		the returned objects list.
 * @param [out] out_wos_metadata	the returned metadata.
 *
 * @return Status.
 */
esdm_status_t esdm_backend_wos_alloc(int       n_dims,
                            int*      dims_size,
                            esdm_type type,
                            char*     md1,
                            char*     md2,
                            void*     w_handle,
			    char**    out_object_id,
                            char**    out_wos_metadata);
/**
 * Close a connection to the WOS cluster
 * 
 * @param [in] wos_handle              The pointer to a wos specific handle
 *
 * @return Status.
 */
esdm_status_t esdm_backend_wos_close(void*    w_handle);

/**
 * Write a data fragment in a WOS object
 * 
 * @param [in] obj_id			Object to be stored with data
 * @param [in] n_dims			number of dimensions.
 * @param [in] dims_size		array of dimensions' size.
 * @param [in] offset			array of dimensions' offset
 * @param [in] type			type of data to be stored.
 * @param [in] w_handle			Pointer to a wos specific handle.
 * @param [in] data			The pointer to a contiguous memory region that shall be written
 * @param [out] out_wos_metadata	the returned metadata.
 *
 * @return Status.
 */
esdm_status_t esdm_backend_wos_append(char*     obj_id,
                            int       n_dims,
                            int*      dims_size,
                            int*      offset,
                            esdm_type type,
                            void*     w_handle,
                            void*     data,
                            char**    out_wos_metadata);


#endif
