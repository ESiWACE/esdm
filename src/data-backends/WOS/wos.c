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

/**
 * @file
 * @brief A data backend to provide WOS compatibility.
 */

#include <esdm.h>
#include <wos.h>

#define WOS_CLUSTER_IP	"192.168.139.147"	// It will be taken from config file
#define OBJ_NUM 	1			// Fixed. It will come from performance evaluation
#define POLICY		"test"			// It will be taken from config file

esdm_status_t esdm_backend_wos_open (void**   wos_handle)
{
	esdm_backend_wos	    *handle;

	if (!(handle = (esdm_backend_wos *) calloc (1, sizeof (esdm_backend_wos)))){
		return ESDM_ERROR;
	}

	/* Establish connection to WOS cluster */
	handle->wos_clutser = Conn(WOS_CLUSTER_IP);
	if(handle->wos_cluster == NULL){
                // print error
                free(handle);
		handle = NULL;
                return ESDM_ERROR;
        }

	handle->wos_policy = GetPolicy(wos_cluster, POLICY);

	if(wos_policy==NULL){
                free(handle);
		handle = NULL;
                return ESDM_ERROR;
        }

	*wos_handle = handle;

	return ESDM_SUCCESS;
}

esdm_status_t esdm_backend_wos_alloc(int       n_dims,
                            int*      dims_size,
                            esdm_type type,
                            char*     md1,
                            char*     md2,
                            void*     w_handle,
                            char**    out_object_id,
                            char**    out_wos_metadata)
{
	
	long num_value	= 1;
	long size 	= 1;
	int  obj_num	= 0;
	int  obj_size	= 0;
	int  obj_offset = 0;
	int  rc		= 0;

	esdm_backend_wos* wos_handle;

	t_WosStatus* wos_status;
	//t_WosOID** oid_list;

	if(!w_handle){
                return ESDM_ERROR;
	}

	wos_handle = (esdm_backend_wos*) w_handle;

	if(!(wos_handle->wos_cluster) || !(wos_handle->wos_policy)){
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
	}

	if(!dims_size || n_dims < 1){
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
	}


	/* Compute the number and size of the values which will be stored */
	for (i = 0; i < n_dims; i++){
		num_value *= dims_size[i];
	}
	size = num_value * sizeof(type);
	
	/* Define best fragmentation */
	//obj_num = wos_get_fragmentation(num_value, size); 	/* To be completed */
	obj_num = OBJ_NUM;


	obj_size = size/obj_num;
	obj_offset = size%obj_num;

	wos_status = CreateStatus();
	if(wos_status==NULL){
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
	}


	/* Create object for internal use */

	rc = wos_metaobj_create (wos_handle->wos_meta_obj, md1, md2); /* To be completed */
	
	/* Reserve oids for storing data objects - null terminated array */

	wos_handle->oid_list = (t_WosOID**)calloc(obj_num + 1, sizeof(t_WosOID*));

	for (i = 0; i < obj_num; i++){
		(wos_handle->oid_list)[i] = CreateWoid();
		if((wos_handle->oid_list)[i] == NULL){
			free(wos_handle->oid_list);
			wos_handle->oid_list = NULL;
                	free(wos_handle);
			wos_handle = NULL;
                	return ESDM_ERROR;	
		}
		Reserve_b(wos_status, (wos_handle->oid_list)[i], wos_handle->policy, wos_handle->cluster);
		if(wos_status == NULL){
                        free(wos_handle->oid_list);
                        wos_handle->oid_list = NULL;
			free(wos_handle);
                        wos_handle = NULL;
                        return ESDM_ERROR;
                }
		

	}

	*out_wos_metadata = object_meta_encode(wos_handle); /* To be completed */
	*out_object_id = object_list_encode(wos_handle->oid_list); /* To be completed */
        
	DeleteWosStatus(wos_status);

	return ESDM_SUCCESS
}

esdm_status_t esdm_backend_wos_close(void*    w_handle)
{
	esdm_backend_wos     *wos_handle;
	int 		     i;

	if(!w_handle){
                return ESDM_ERROR;
	}

	wos_handle = (esdm_backend_wos*) w_handle;

	for (i=0; wos_handle->oid_list[i]; i++){
        	DeleteWosWoid(wos_handle->oid_list[i]);
	}

        DeleteWosCluster(wos_handle->wos_cluster);
        DeleteWosPolicy(wos_handle->wos_policy);

	free(wos_handle);
	return 0;
}

esdm_status_t esdm_backend_wos_append(char*     obj_id,
                            int       n_dims,
                            int*      dims_size,
                            int*      offset,
                            esdm_type type,
                            void*     w_handle,
                            char**    out_object_id,
                            void*     data,
                            char**    out_wos_metadata)
{
	
	long num_value	= 1;
	long size 	= 1;
	int  obj_num	= 0;
	int  obj_size	= 0;
	int  obj_offset = 0;
	int  rc		= 0;

	esdm_backend_wos* wos_handle;

	t_WosStatus* wos_status;

	if(!w_handle){
                return ESDM_ERROR;
	}

	wos_handle = (esdm_backend_wos*) w_handle;

	if(!(wos_handle->wos_cluster) || !(wos_handle->wos_policy)){
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
	}

	if(!dims_size || n_dims < 1){
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
	}

        t_WosObjPtr* wobj = WosObjCreate();
        if(wobj==NULL){
                //printf("Error: No object created");
                return ESDM_ERROR;
        }
     
	/* Compute the number and size of the values which will be stored */
	for (i = 0; i < n_dims; i++){
		
		size*=dims_size[i];
	}
        size=size*sizeof(type);
	
	wos_status = CreateStatus();
	if(wos_status==NULL){
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
	}

        //SetMetadataObj to be defined (e.g. offset, size, ..)
        SetDataObj(data,size, wobj);

        PutOID_b(wos_status, obj_id, wobj, wos_handle->cluster);
        if (GetStatus(wos_status) != 0){
                //printf("Something went wrong with put OID operation\n");
                free(wos_handle);
		wos_handle = NULL;
                return ESDM_ERROR;
        }
	DeleteWosStatus(wos_status);
	DeleteWosObj(wobj);

        return ESDM_SUCCESS;
}

