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

/**
 * @file
 * @brief A data backend to provide Clovis compatibility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <esdm.h>

#include "clovis.h"


#define PAGE_4K (4096ULL)
#define BLOCKSIZE (PAGE_4K)
#define BLOCKMASK (BLOCKSIZE - 1)

static struct m0_uint128 gid;

static inline
esdm_backend_clovis_t *eb2ebm(esdm_backend_t *eb)
{
    return container_of(eb, esdm_backend_clovis_t, ebm_base);
}

static int conf_parse(char * conf, esdm_backend_clovis_t *ebm)
{
    /*
     * Parse the conf string into ebm->ebm_clovis_conf.
     */
    static char *clovis_local_addr;
    static char *clovis_ha_addr;
    static char *clovis_prof;
    static char *clovis_proc_fid;
    static char *clovis_index_dir = "/tmp/";

    if ((clovis_local_addr = strsep(&conf, " ")) == NULL) {
        return -EINVAL;
    }
    ebm->ebm_clovis_conf.cc_local_addr = strdup(clovis_local_addr);
    if ((clovis_ha_addr = strsep(&conf, " ")) == NULL) {
        return -EINVAL;
    }
    ebm->ebm_clovis_conf.cc_ha_addr = strdup(clovis_ha_addr);
    if ((clovis_prof = strsep(&conf, " ")) == NULL) {
        return -EINVAL;
    }
    ebm->ebm_clovis_conf.cc_profile = strdup(clovis_prof);
    if ((clovis_proc_fid = strsep(&conf, " ")) == NULL) {
        return -EINVAL;
    }
    ebm->ebm_clovis_conf.cc_process_fid = strdup(clovis_proc_fid);

    ebm->ebm_clovis_conf.cc_is_oostore            = false;
    ebm->ebm_clovis_conf.cc_is_read_verify        = false;
    ebm->ebm_clovis_conf.cc_tm_recv_queue_min_len = M0_NET_TM_RECV_QUEUE_DEF_LEN;
    ebm->ebm_clovis_conf.cc_max_rpc_msg_size      = M0_RPC_DEF_MAX_RPC_MSG_SIZE;
    ebm->ebm_clovis_conf.cc_idx_service_id        = M0_CLOVIS_IDX_MOCK;
    ebm->ebm_clovis_conf.cc_idx_service_conf      = clovis_index_dir;
    ebm->ebm_clovis_conf.cc_layout_id             = 0;

    printf("local addr = %s\n", ebm->ebm_clovis_conf.cc_local_addr);
    printf("ha addr = %s\n",    ebm->ebm_clovis_conf.cc_ha_addr);
    printf("profile = %s\n",    ebm->ebm_clovis_conf.cc_profile);
    printf("process id = %s\n", ebm->ebm_clovis_conf.cc_process_fid);
    return 0;
}

int esdm_backend_clovis_init(char * conf, esdm_backend_t *eb)
{
	esdm_backend_clovis_t *ebm;
	int                       rc;

    ebm = eb2ebm(eb);

	rc = conf_parse(conf, ebm);
	if (rc != 0) {
		return rc;
	}

    /* Clovis instance */
    rc = m0_clovis_init(&ebm->ebm_clovis_instance, &ebm->ebm_clovis_conf, true);
    if (rc != 0) {
        printf("Failed to initilise Clovis\n");
		return rc;
    }

    /* And finally, clovis root realm */
    m0_clovis_container_init(&ebm->ebm_clovis_container,
                             NULL, &M0_CLOVIS_UBER_REALM,
                             ebm->ebm_clovis_instance);
    rc = ebm->ebm_clovis_container.co_realm.re_entity.en_sm.sm_rc;

    if (rc != 0) {
        printf("Failed to open uber realm\n");
        return rc;
    }

    gid = M0_CLOVIS_ID_APP;
    /* FIXME this makes the gid not reused. */
    gid.u_hi = gid.u_lo = time(NULL);
	return 0;
}

int esdm_backend_clovis_fini(esdm_backend_t *eb)
{
	esdm_backend_clovis_t *ebm;

	ebm = eb2ebm(eb);
    m0_clovis_fini(ebm->ebm_clovis_instance, true);
    free((char*)ebm->ebm_clovis_conf.cc_local_addr);
    free((char*)ebm->ebm_clovis_conf.cc_ha_addr);
    free((char*)ebm->ebm_clovis_conf.cc_profile);
    free((char*)ebm->ebm_clovis_conf.cc_process_fid);
	free(ebm);
	return 0;
}

static void open_entity(struct m0_clovis_obj *obj)
{
	struct m0_clovis_entity *entity = &obj->ob_entity;
	struct m0_clovis_op     *ops[1] = { NULL };

	m0_clovis_entity_open(entity, &ops[0]);
	m0_clovis_op_launch(ops, 1);
	m0_clovis_op_wait(ops[0], M0_BITS(M0_CLOVIS_OS_FAILED, M0_CLOVIS_OS_STABLE),
                      m0_time_from_now(3,0));
	m0_clovis_op_fini(ops[0]);
	m0_clovis_op_free(ops[0]);
	ops[0] = NULL;
}

static int create_object(esdm_backend_clovis_t *ebm,
                         struct m0_uint128 id)
{
	int                  rc = 0;
	struct m0_clovis_obj obj;
	struct m0_clovis_op *ops[1] = {NULL};

	memset(&obj, 0, sizeof(struct m0_clovis_obj));

	m0_clovis_obj_init(&obj, &ebm->ebm_clovis_container.co_realm,
	                   &id,
	                   m0_clovis_default_layout_id(ebm->ebm_clovis_instance));

	open_entity(&obj);

	m0_clovis_entity_create(&obj.ob_entity, &ops[0]);

	m0_clovis_op_launch(ops, ARRAY_SIZE(ops));

	rc = m0_clovis_op_wait(ops[0],
                           M0_BITS(M0_CLOVIS_OS_FAILED, M0_CLOVIS_OS_STABLE),
                           m0_time_from_now(3,0));

	m0_clovis_op_fini(ops[0]);
	m0_clovis_op_free(ops[0]);
	m0_clovis_obj_fini(&obj);

	return rc;
}

/**
 * We need a mechansim to allocate id globally, uniquely.
 * We may need to store the last allocated id in store and
 * read it on start.
 */
static struct m0_uint128 object_id_alloc()
{
    /* gid.u_hi keeps unchanged in a one session. */
    gid.u_lo++;
    return gid;
}

/**
 * A memroy of OBJECT_NAME_LENGTH length is allocated, and
 * the caller needs to free it after use.
 */
static char* object_id_encode(const struct m0_uint128 *obj_id)
{
    enum { OBJECT_NAME_LENGTH = 64};
    /* "oid=<0x1234567812345678:0x1234567812345678>" */
    char *json = malloc(OBJECT_NAME_LENGTH);
    struct m0_fid fid;

    fid.f_container = obj_id->u_hi;
    fid.f_key = obj_id->u_lo;

    if (json != NULL) {
        memset(json, 0, OBJECT_NAME_LENGTH);
        snprintf(json, OBJECT_NAME_LENGTH, "oid="FID_F, FID_P(&fid));
    }
    return json;
}

static int object_id_decode(char *oid_json, struct m0_uint128 *obj_id)
{
    struct m0_fid fid;
    int           rc;
    char          *oid = strchr(oid_json, '=');

    if (oid == NULL)
        return -EINVAL;

    oid++;
    rc = m0_fid_sscanf(oid, &fid);
    if (rc != 0)
        return rc;

    obj_id->u_hi = fid.f_container;
    obj_id->u_lo = fid.f_key;

    return 0;
}

static char* object_meta_encode(const struct m0_uint128 *obj_id)
{
    return NULL;
}

int esdm_backend_clovis_alloc(esdm_backend_t *eb,
                            int       n_dims,
                            int*      dims_size,
                            esdm_type type,
                            char*     md1,
			                char*     md2,
			                char**    out_object_id,
			                char**    out_mero_metadata)
{
	esdm_backend_clovis_t *ebm;
	struct m0_uint128         obj_id;
	int                       rc;

	ebm = eb2ebm(eb);

	/* First step: alloc a new fid for this new object. */
	obj_id = object_id_alloc();
	printf("new obj id = "FID_F"\n", FID_P((struct m0_fid*)&obj_id));

	/* Then create object */
	rc = create_object(ebm, obj_id);
	if (rc == 0) {
		/* encode this obj_id into string */
		*out_object_id = object_id_encode(&obj_id);
		*out_mero_metadata = object_meta_encode(&obj_id);
	}
	return rc;
}

int esdm_backend_clovis_open (esdm_backend_t *eb,
	                        char*     object_id,
			                void**    obj_handle)
{
	esdm_backend_clovis_t *ebm;
	struct m0_clovis_obj     *obj;
	struct m0_uint128         obj_id;
	int                       rc = 0;

	ebm = eb2ebm(eb);

	/* convert from json string to object id. */
	object_id_decode(object_id, &obj_id);

	obj = malloc(sizeof *obj);
	if (obj == NULL)
		return -ENOMEM;

	memset(obj, 0, sizeof(struct m0_clovis_obj));
	m0_clovis_obj_init(obj, &ebm->ebm_clovis_container.co_realm,
			   &obj_id,
			   m0_clovis_default_layout_id(ebm->ebm_clovis_instance));

	open_entity(obj);
	*obj_handle = obj;

	return rc;
}

int esdm_backend_clovis_rdwr (esdm_backend_t *eb,
                            void*    obj_handle,
                            uint64_t start,
                            uint64_t count,
                            void*    data,
                            int      rdwr_op)
{
	struct m0_clovis_obj     *obj = (struct m0_clovis_obj*)obj_handle;
    uint64_t                  i;
	struct m0_clovis_op      *ops[1] = {NULL};
	struct m0_indexvec        ext;
	struct m0_bufvec          data_buf;
	struct m0_bufvec          attr_buf;
    uint64_t                  clovis_block_count;
    uint64_t                  clovis_block_size;
    int                       rc;

	assert((start & BLOCKMASK) == 0);
	assert(((start + count) & BLOCKMASK) == 0);
    assert(rdwr_op == M0_CLOVIS_OC_READ || rdwr_op == M0_CLOVIS_OC_WRITE);

	/*
	 * read the extended region and copy the data from new buffers.
	 */

    clovis_block_size = BLOCKSIZE;
    clovis_block_count = count / clovis_block_size;

	/* we want to read <clovis_block_count> from @start of the object */
	rc = m0_indexvec_alloc(&ext, clovis_block_count);
	if (rc != 0)
		return rc;

	/*
	 * this allocates <clovis_block_count> empty buffers for data.
	 */
	rc = m0_bufvec_empty_alloc(&data_buf, clovis_block_count);
	if (rc != 0) {
        m0_indexvec_free(&ext);
		return rc;
    }

	rc = m0_bufvec_alloc(&attr_buf, clovis_block_count, 1);
	if(rc != 0) {
        m0_indexvec_free(&ext);
        m0_bufvec_free2(&data_buf);
		return rc;
    }

	for (i = 0; i < clovis_block_count; i++) {
		ext.iv_index[i]            = start + clovis_block_size * i;
		ext.iv_vec.v_count[i]      = clovis_block_size;

        data_buf.ov_buf[i]         = data  + clovis_block_size * i;
        data_buf.ov_vec.v_count[i] = clovis_block_size;

		/* we don't want any attributes */
		attr_buf.ov_vec.v_count[i] = 0;
	}

	/* Create the read request */
	m0_clovis_obj_op(obj, rdwr_op, &ext, &data_buf,
                     &attr_buf, 0, &ops[0]);
	M0_ASSERT(rc == 0);
	M0_ASSERT(ops[0] != NULL);
	M0_ASSERT(ops[0]->op_sm.sm_rc == 0);

	m0_clovis_op_launch(ops, 1);

	/* wait */
	rc = m0_clovis_op_wait(ops[0],
	                       M0_BITS(M0_CLOVIS_OS_FAILED,
                                   M0_CLOVIS_OS_STABLE),
	                       M0_TIME_NEVER);
	M0_ASSERT(rc == 0);
	M0_ASSERT(ops[0]->op_sm.sm_state == M0_CLOVIS_OS_STABLE);
	M0_ASSERT(ops[0]->op_sm.sm_rc == 0);

	/* fini and release */
	m0_clovis_op_fini(ops[0]);
	m0_clovis_op_free(ops[0]);

	m0_indexvec_free(&ext);
	m0_bufvec_free2(&data_buf);
	m0_bufvec_free(&attr_buf);

    return rc;
}

int esdm_backend_clovis_write(esdm_backend_t *eb,
                            void*    obj_handle,
                            uint64_t start,
                            uint64_t count,
                            void*    data)
{
    int rc;
	assert((start & BLOCKMASK) == 0);
	assert(((start + count) & BLOCKMASK) == 0);
    rc = esdm_backend_clovis_rdwr(eb, obj_handle, start, count,
                                data, M0_CLOVIS_OC_WRITE);
    return rc;
}

int esdm_backend_clovis_read (esdm_backend_t *eb,
                            void*    obj_handle,
                            uint64_t start,
                            uint64_t count,
                            void*    data)
{
    int rc;
	assert((start & BLOCKMASK) == 0);
	assert(((start + count) & BLOCKMASK) == 0);
    rc = esdm_backend_clovis_rdwr(eb, obj_handle, start, count,
                                data, M0_CLOVIS_OC_READ);
    return rc;
}

int esdm_backend_clovis_close(esdm_backend_t *eb,
                            void                        *obj_handle)
{
	struct m0_clovis_obj     *obj;
    int                       rc = 0;

	obj = (struct m0_clovis_obj*)obj_handle;
	m0_clovis_obj_fini(obj);
	free(obj);

	return rc;
}

int clovis_backend_performance_estimate()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
esdm_backend_clovis_t esdm_backend_clovis = {
    .ebm_base = {
	    .name      = "CLOVIS",
    	.type      = ESDM_TYPE_DATA,
    	.version   = "0.0.1",
    	.options   = NULL,
    	.data      = NULL,
        .blocksize = BLOCKSIZE,
        .callbacks = {
    		(int (*)())esdm_backend_clovis_fini, // finalize
    		clovis_backend_performance_estimate, // performance_estimate

    		(int (*)())esdm_backend_clovis_alloc,
    		(int (*)())esdm_backend_clovis_open,
    		(int (*)())esdm_backend_clovis_write,
    		(int (*)())esdm_backend_clovis_read,
    		(int (*)())esdm_backend_clovis_close,

			// Metadata Callbacks
			NULL, // lookup

			// ESDM Data Model Specific
			NULL, // container create
			NULL, // container retrieve
			NULL, // container update
			NULL, // container delete

			NULL, // dataset create
			NULL, // dataset retrieve
			NULL, // dataset update
			NULL, // dataset delete

			NULL, // fragment create
			NULL, // fragment retrieve
			NULL, // fragment update
			NULL, // fragment delete
    	},
    },
    .ebm_ops = {
    	.esdm_backend_init      = esdm_backend_clovis_init,
    	.esdm_backend_fini      = esdm_backend_clovis_fini,

    	.esdm_backend_obj_alloc = esdm_backend_clovis_alloc,
    	.esdm_backend_obj_open  = esdm_backend_clovis_open,
    	.esdm_backend_obj_write = esdm_backend_clovis_write,
    	.esdm_backend_obj_read  = esdm_backend_clovis_read,
    	.esdm_backend_obj_close = esdm_backend_clovis_close
    },
};

/**
* Initializes the CLOVIS plugin. In particular this involves:
*
*	* Load configuration of this backend
*	* Load and potenitally calibrate performance model
*
*	* Connect with support services e.g. for technical metadata
*	* Setup directory structures used by this CLOVIS specific backend
*
*	* Poopulate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/
esdm_backend_t* clovis_backend_init()
{
    esdm_backend_t *eb = &esdm_backend_clovis.ebm_base;
    int rc;

    rc = esdm_backend_clovis_init("conf string", eb);

    if (rc != 0)
        return NULL;
    else
	    return eb;
}
