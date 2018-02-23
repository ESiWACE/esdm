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
 */

/**
 * @file
 * @brief A data backend to provide wos compatibility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esdm.h>

#include "wos.h"

#define PAGE_4K (4096ULL)
#define BLOCKSIZE (PAGE_4K)
#define BLOCKMASK (BLOCKSIZE - 1)

#define WOS_HOST "host"
#define WOS_POLICY "policy"
#define WOS_OBJ_NUM 1

// Temporary definitions
#define ESDM_DEBUG(msg) fprintf(stderr, "[%s][%d] %s\n", __FILE__, __LINE__, msg)

int wos_sizeof(esdm_type type)
{
	// Return the size (in bytes) corresponding to data type 'type'
	// TODO: change the reference to correct core function
	return (int)type;
}

int wos_get_param(const char *conf, char **output, const char *param)
{
	if (!conf || !output) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}
	*output = NULL;

	char *key = strstr(conf, param);
	if (!key) {
		ESDM_DEBUG("Parameter not found");
		return 1;
	}
	char *value = strchr(key, '=');
	if (!value || !*value) {
		ESDM_DEBUG("Parameter not found");
		return 1;
	}
	value++;
	char *end_value = strchr(value, ';');
	if (!end_value) {
		ESDM_DEBUG("Parameter not found");
		return 1;
	}
	size_t size = end_value - value;
	char _output[1 + size];
	strncpy(_output, value, size);
	_output[size] = 0;
	*output = strdup(_output);
	if (!*output) {
		ESDM_DEBUG("Memory error");
		return 1;
	}

	return 0;
}

int wos_get_host(const char *conf, char **host)
{
	return wos_get_param(conf, host, WOS_HOST);
}

int wos_get_policy(const char *conf, char **policy)
{
	return wos_get_param(conf, policy, WOS_POLICY);
}

void wos_delete_oid_list(esdm_backend_wos_t * ebm)
{
	if (ebm->oid_list) {
		int j = 0;
		while (ebm->oid_list[j])
			DeleteWosWoid(ebm->oid_list[j++]);
		free(ebm->oid_list);
		ebm->oid_list = NULL;
	}
	if (ebm->size_list) {
		free(ebm->size_list);
		ebm->size_list = NULL;
	}
}

int wos_object_list_encode(t_WosOID ** oid_list, char **out_object_id)
{
	if (!out_object_id) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}
	*out_object_id = NULL;

	if (oid_list) {
		int i;
		const char *current;
		for (i = 0; oid_list[i]; ++i) {
			current = OIDtoString(oid_list[i]);
			if (*out_object_id) {
				char buffer[strlen(*out_object_id) + strlen(current) + 2];
				sprintf(buffer, "%s,%s", *out_object_id, current);
				free(*out_object_id);
				*out_object_id = strdup(buffer);
			} else
				*out_object_id = strdup(current);
			if (!out_object_id) {
				ESDM_DEBUG("Memory error");
				return 1;
			}
		}
	}

	return 0;
}

int esdm_backend_wos_init(char *conf, esdm_backend_t * eb)
{
	esdm_backend_wos_t *ebm = (esdm_backend_wos_t *) eb;
	if (!ebm) {
		ESDM_DEBUG("Unable to get struct");
		return 1;
	}

	ebm->wos_cluster = NULL;
	ebm->wos_policy = NULL;
	ebm->wos_meta_obj = NULL;
	ebm->oid_list = NULL;

	char *host = NULL;
	if (wos_get_host(conf, &host) || !host) {
		ESDM_DEBUG("Unable to get host address");
		return 1;
	}
	ebm->wos_cluster = Conn(host);
	if (!ebm->wos_cluster) {
		free(host);
		ESDM_DEBUG("Unable to connect to cluster");
		return 1;
	}
	free(host);

	char *policy = NULL;
	if (wos_get_policy(conf, &policy) || !policy) {
		DeleteWosCluster(ebm->wos_cluster);
		ebm->wos_cluster = NULL;
		ESDM_DEBUG("Unable to get policy name");
		return 1;
	}
	ebm->wos_policy = GetPolicy(ebm->wos_cluster, policy);
	if (!ebm->wos_policy) {
		DeleteWosCluster(ebm->wos_cluster);
		ebm->wos_cluster = NULL;
		free(policy);
		ESDM_DEBUG("Unable to get policy");
		return 1;
	}
	free(policy);

	return 0;
}

int esdm_backend_wos_fini(esdm_backend_t * eb)
{
	esdm_backend_wos_t *ebm = (esdm_backend_wos_t *) eb;
	if (!ebm) {
		ESDM_DEBUG("Unable to get struct");
		return 1;
	}

	int i;
	if (ebm->oid_list) {
		for (i = 0; ebm->oid_list[i]; i++) {
			DeleteWosWoid(ebm->oid_list[i]);
		}
		free(ebm->oid_list);
	}

	if (ebm->wos_cluster)
		DeleteWosCluster(ebm->wos_cluster);
	if (ebm->wos_policy)
		DeleteWosPolicy(ebm->wos_policy);

	return 0;
}

int esdm_backend_wos_alloc(esdm_backend_t * eb, int n_dims, int *dims_size, esdm_type type, char *md1, char *md2, char **out_object_id, char **out_wos_metadata)
{
	if (!out_object_id || !out_wos_metadata) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}
	*out_object_id = NULL;
	*out_wos_metadata = NULL;

	int item_size = wos_sizeof(type);
	if (!dims_size || (n_dims < 1) || !item_size) {
		ESDM_DEBUG("Wrong parameters");
		return 1;
	}

	esdm_backend_wos_t *ebm = (esdm_backend_wos_t *) eb;
	if (!ebm) {
		ESDM_DEBUG("Unable to get struct");
		return 1;
	}
	if (!ebm->wos_cluster || !ebm->wos_policy) {
		ESDM_DEBUG("Unable to get wos parameters");
		return 1;
	}

	t_WosStatus *wos_status = CreateStatus();
	if (!wos_status) {
		ESDM_DEBUG("Unable to create wos status");
		return 1;
	}

	int i;
	uint64_t num_value = 1;
	for (i = 0; i < n_dims; i++)
		num_value *= dims_size[i];
	uint64_t size = num_value * item_size;

	/* Set the best fragmentation */
	//obj_num = wos_get_fragmentation(num_value, size);     /* To be completed */
	int obj_num = WOS_OBJ_NUM;
	uint64_t obj_size = size / obj_num;
	uint64_t obj_offset = size % obj_num;

	/* Create object for internal use */
	//rc = wos_metaobj_create(&ebm->wos_meta_obj, md1, md2);        /* To be completed */

	/* Reserve oids for storing data objects - null terminated array */
	ebm->oid_list = (t_WosOID **) calloc(obj_num + 1, sizeof(t_WosOID *));
	ebm->size_list = (uint64_t *) calloc(obj_num + 1, sizeof(uint64_t));

	for (i = 0; i < obj_num; i++) {
		ebm->oid_list[i] = CreateWoid();
		if (!ebm->oid_list[i]) {
			wos_delete_oid_list(ebm);
			DeleteWosStatus(wos_status);
			ESDM_DEBUG("Unable to create the objects");
			return 1;
		}
		ebm->size_list[i] = obj_offset && (i == (obj_num - 1)) ? obj_offset : obj_size;
		Reserve_b(wos_status, ebm->oid_list[i], ebm->wos_policy, ebm->wos_cluster);
		if (GetStatus(wos_status)) {
			wos_delete_oid_list(ebm);
			DeleteWosStatus(wos_status);
			ESDM_DEBUG("Unable to create the objects");
			return 1;
		}
	}

	if (wos_object_list_encode(ebm->oid_list, out_object_id)) {
		wos_delete_oid_list(ebm);
		DeleteWosStatus(wos_status);
		ESDM_DEBUG("Unable to set output data");
		return 1;
	}
	//wos_object_meta_encode(ebm, out_wos_metadata);  /* To be completed */

	DeleteWosStatus(wos_status);

	return 0;
}

int esdm_backend_wos_open(esdm_backend_t * eb, char *object_id, void **obj_handle)
{
	if (!object_id || !obj_handle) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}
	*obj_handle = NULL;

	esdm_backend_wos_t *ebm = (esdm_backend_wos_t *) eb;
	if (!ebm) {
		ESDM_DEBUG("Unable to get struct");
		return 1;
	}
	if (!ebm->wos_cluster || !ebm->wos_policy) {
		ESDM_DEBUG("Unable to get wos parameters");
		return 1;
	}

	t_WosStatus *wos_status = CreateStatus();
	if (!wos_status) {
		ESDM_DEBUG("Unable to create wos status");
		return 1;
	}

	t_WosOID *oid = CreateWoid();
	if (!oid) {
		DeleteWosStatus(wos_status);
		ESDM_DEBUG("Unable to create a wos oid");
		return 1;
	}
	SetOID(oid, object_id);

/*
	Exists_b(wos_status, oid, ebm->wos_cluster);
	if (GetStatus(wos_status)) {
		DeleteWosWoid(oid);
		DeleteWosStatus(wos_status);
		ESDM_DEBUG("Unable to find the object");
		return 1;
	}
*/

	*obj_handle = (void *)oid;

	DeleteWosStatus(wos_status);

	return 0;
}

int esdm_backend_wos_write(esdm_backend_t * eb, void *obj_handle, uint64_t start, uint64_t count, void *data)
{
	if (!obj_handle) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}
	if (!data || !count) {
		ESDM_DEBUG("No data has to be written");
		return 1;
	}

	esdm_backend_wos_t *ebm = (esdm_backend_wos_t *) eb;
	if (!ebm) {
		ESDM_DEBUG("Unable to get struct");
		return 1;
	}
	if (!ebm->wos_cluster || !ebm->wos_policy) {
		ESDM_DEBUG("Unable to get wos parameters");
		return 1;
	}
	if (!ebm->size_list || !ebm->size_list[0]) {
		ESDM_DEBUG("Unable to get object size");
		return 1;
	}
	uint64_t obj_size = ebm->size_list[0];

	esdm_type type = 1; // TODO: data type should be extracted from metadata associated to WOS object
	int item_size = wos_sizeof(type);

	// TODO: start and count has to be used to write the appropriate objects; only one object is now created
	start *=  item_size;
	count *=  item_size;
	if (obj_size < start + count) {
		ESDM_DEBUG("Wrong parameters start or count");
		return 1;
	}

	t_WosStatus *wos_status = CreateStatus();
	if (!wos_status) {
		ESDM_DEBUG("Unable to create wos status");
		return 1;
	}

	t_WosObjPtr *wobj = NULL;
	void *buffer = calloc(obj_size, sizeof(char));
	if (!buffer) {
		DeleteWosStatus(wos_status);
		ESDM_DEBUG("Memory error");
		return 1;
	}

	int i;
	for (i = 0; ebm->oid_list[i]; i++) {

		wobj = WosObjCreate();
		if (!wobj) {
			DeleteWosStatus(wos_status);
			ESDM_DEBUG("Unable to create wos object");
			return 1;
		}
		ESDM_DEBUG("New object created");

		/* Fill data */
		// TODO: start and count has to be used to write the appropriate objects; only one object is now created
		bzero(buffer, ebm->size_list[i]);
		memcpy(buffer + start, data, count);
		SetDataObj(buffer, ebm->size_list[i], wobj);

		PutOID_b(wos_status, ebm->oid_list[i], wobj, ebm->wos_cluster);
		if (GetStatus(wos_status)) {
			DeleteWosStatus(wos_status);
			ESDM_DEBUG("Unable to allocate data");
			return 1;
		}

		DeleteWosObj(wobj);
		ESDM_DEBUG("Hook removed");
	}

	free(buffer);

	DeleteWosStatus(wos_status);

	return 0;
}

int esdm_backend_wos_read(esdm_backend_t * eb, void *obj_handle, uint64_t start, uint64_t count, void *data)
{
	if (!obj_handle) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}
	if (!data || !count) {
		ESDM_DEBUG("No data has to be read");
		return 1;
	}

	esdm_backend_wos_t *ebm = (esdm_backend_wos_t *) eb;
	if (!ebm) {
		ESDM_DEBUG("Unable to get struct");
		return 1;
	}
	if (!ebm->wos_cluster || !ebm->wos_policy) {
		ESDM_DEBUG("Unable to get wos parameters");
		return 1;
	}

	t_WosStatus *wos_status = CreateStatus();
	if (!wos_status) {
		ESDM_DEBUG("Unable to create wos status");
		return 1;
	}

	t_WosObjPtr *wobj = WosObjHookCreate();
	if (!wobj) {
		DeleteWosStatus(wos_status);
		ESDM_DEBUG("Unable to create wos object");
		return 1;
	}
	ESDM_DEBUG("New hook created");

	Get_b(wos_status, (t_WosOID *)obj_handle, wobj, ebm->wos_cluster);
	if (GetStatus(wos_status)) {
		DeleteWosObj(wobj);
		ESDM_DEBUG("Hook removed");
		DeleteWosStatus(wos_status);
		ESDM_DEBUG("Unable to allocate data");
		return 1;
	}

	DeleteWosStatus(wos_status);

	const void *buffer = NULL;
	int len;
	GetDataObj(&buffer, &len, wobj);
	if (!buffer || !len) {
		DeleteWosObj(wobj);
		ESDM_DEBUG("Hook removed");
		ESDM_DEBUG("Unable to get data");
		return 1;
	}

	esdm_type type = 1; // TODO: data type should be extracted from metadata associated to WOS object
	int item_size = wos_sizeof(type);

	// TODO: start and count has to be used to select the appropriate objects, get data and aggregate the result to be outputed; now there is only one object
	start *=  item_size;
	count *=  item_size;
	if (len < start + count) {
		DeleteWosObj(wobj);
		ESDM_DEBUG("Hook removed");
		ESDM_DEBUG("Wrong parameters start or count");
		return 1;
	}

	memcpy(data + start, buffer, count);

	DeleteWosObj(wobj);
	ESDM_DEBUG("Hook removed");

	return 0;
}

int esdm_backend_wos_close(esdm_backend_t * eb, void *obj_handle)
{
	if (!obj_handle) {
		ESDM_DEBUG("Null pointer");
		return 1;
	}

	DeleteWosWoid((t_WosOID *)obj_handle);

	return 0;
}

int wos_backend_performance_estimate()
{
	ESDM_DEBUG("Not implemented");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
esdm_backend_wos_t esdm_backend_wos = {
	.ebm_base = {
		     .name = "wos",
		     .type = ESDM_TYPE_DATA,
		     .version = "0.0.1",
		     .data = NULL,
		     .blocksize = BLOCKSIZE,
		     .callbacks = {
				   (int (*)()) esdm_backend_wos_fini,	// finalize
				   wos_backend_performance_estimate,	// performance_estimate

				   (int (*)()) esdm_backend_wos_alloc,
				   (int (*)()) esdm_backend_wos_open,
				   (int (*)()) esdm_backend_wos_write,
				   (int (*)()) esdm_backend_wos_read,
				   (int (*)()) esdm_backend_wos_close,

				   NULL,	// allocate
				   NULL,	// update
				   NULL,	// lookup
				   },
		     },
	.ebm_ops = {
		    .esdm_backend_init = esdm_backend_wos_init,
		    .esdm_backend_fini = esdm_backend_wos_fini,

		    .esdm_backend_obj_alloc = esdm_backend_wos_alloc,
		    .esdm_backend_obj_open = esdm_backend_wos_open,
		    .esdm_backend_obj_write = esdm_backend_wos_write,
		    .esdm_backend_obj_read = esdm_backend_wos_read,
		    .esdm_backend_obj_close = esdm_backend_wos_close},
};

/**
* Initializes the plugin. In particular this involves:
*
*	* Load configuration of this backend
*	* Load and potenitally calibrate performance model
*
*	* Connect with support services e.g. for technical metadata
*	* Setup directory structures used by this wos specific backend
*
*	* Popopulate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/
esdm_backend_t *wos_backend_init()
{
	esdm_backend_t *eb = &esdm_backend_wos.ebm_base;
	int rc;

	rc = esdm_backend_wos_init("host=127.0.0.1;policy=test;", eb);

	if (rc != 0)
		return NULL;
	else
		return eb;
}
