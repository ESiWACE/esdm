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
#include "clovis.h"

#define PAGE_4K (4096ULL)
#define BLOCKSIZE (PAGE_4K)
#define BLOCKMASK (BLOCKSIZE - 1)

static int conf_parse(char * conf, struct esdm_backend_mero *ebm)
{
    /*
     * Parse the conf string into ebm->ebm_clovis_conf.
     */
    return 0;
}

int esdm_backend_mero_init(char * conf, struct esdm_backend_generic **eb_out)
{
	struct esdm_backend_mero *ebm;
	int                       rc;

	ebm = malloc(sizeof *ebm);
	if (ebm == NULL)
		return -ENOMEM;

	rc = conf_parse(conf, ebm);
	if (rc != 0) {
		free(ebm);
		return rc;
	}

    /* Clovis instance */
    rc = m0_clovis_init(&ebm->ebm_clovis_instance, &ebm->ebm_clovis_conf, true);
    if (rc != 0) {
        printf("Failed to initilise Clovis\n");
        free(ebm);
		return rc;
    }

    /* And finally, clovis root realm */
    m0_clovis_container_init(&ebm->ebm_clovis_container,
                             NULL, &M0_CLOVIS_UBER_REALM,
                             ebm->ebm_clovis_instance);
    rc = ebm->ebm_clovis_container.co_realm.re_entity.en_sm.sm_rc;

    if (rc != 0) {
        printf("Failed to open uber realm\n");
        free(ebm);
        return rc;
    }

	*eb_out = &ebm->ebm_base;
	return 0;
}

static inline
struct esdm_backend_mero *eb2ebm(struct esdm_backend_generic *eb)
{
    return container_of(eb, struct esdm_backend_mero, ebm_base);
}

int esdm_backend_mero_fini(struct esdm_backend_generic *eb)
{
	struct esdm_backend_mero *ebm;

	ebm = eb2ebm(eb);
	m0_clovis_fini(ebm->ebm_clovis_instance, true);
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

static int create_object(struct esdm_backend_mero *ebm,
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

static struct m0_uint128 object_id_alloc()
{
    static struct m0_uint128 gid;

    gid.u_lo++;
    return gid;
}

static char* object_id_encode(const struct m0_uint128 *obj_id)
{
    enum { OBJECT_NAME_LENGTH = 64};
    char *json = malloc(OBJECT_NAME_LENGTH);

    if (json != NULL) {
        memset(json, 0, OBJECT_NAME_LENGTH);
        snprintf(json, OBJECT_NAME_LENGTH, "oid='%llu:%llu'",
                 (unsigned long long)obj_id->u_hi,
                 (unsigned long long)obj_id->u_lo);
    }
    return json;
}

static int object_id_decode(char * oid_json, struct m0_uint128 *obj_id)
{

    return 0;
}

static char* object_meta_encode(const struct m0_uint128 *obj_id)
{
    return NULL;
}

int esdm_backend_mero_alloc(struct esdm_backend_generic *eb,
                            int       n_dims,
                            int*      dims_size,
                            esdm_type type,
                            char*     md1,
			                char*     md2,
			                char**    out_object_id,
			                char**    out_mero_metadata)
{
	struct esdm_backend_mero *ebm;
	struct m0_uint128         obj_id;
	int                       rc;

	ebm = eb2ebm(eb);

	/* First step: alloc a new fid for this new object. */
	obj_id = object_id_alloc();

	/* Then create object */
	rc = create_object(ebm, obj_id);
	if (rc == 0) {
		/* encode this obj_id into string */
		*out_object_id = object_id_encode(&obj_id);
		*out_mero_metadata = object_meta_encode(&obj_id);
	}
	return rc;
}

int esdm_backend_mero_open (struct esdm_backend_generic *eb,
	                        char*     object_id,
			                void**    obj_handle)
{
	struct esdm_backend_mero *ebm;
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

int esdm_backend_mero_rdwr (struct esdm_backend_generic *eb,
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
	if (rc != 0)
		return rc;

	rc = m0_bufvec_alloc(&attr_buf, clovis_block_count, 1);
	if(rc != 0)
		return rc;

	for (i = 0; i < clovis_block_count; i++) {
		ext.iv_index[i]            = start + clovis_block_size * i;
		ext.iv_vec.v_count[i]      = clovis_block_size;

        data_buf.ov_buf[i]         = data  + clovis_block_size * i;
        data_buf.ov_vec.v_count[i] = clovis_block_size;

		/* we don't want any attributes */
		attr_buf.ov_vec.v_count[i] = 0;
	}

	/* Create the read request */
	m0_clovis_obj_op(obj, M0_CLOVIS_OC_READ, &ext, &data_buf,
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

int esdm_backend_mero_write(struct esdm_backend_generic *eb,
                            void*    obj_handle,
                            uint64_t start,
                            uint64_t count,
                            void*    data)
{
    int rc;
	assert((start & BLOCKMASK) == 0);
	assert(((start + count) & BLOCKMASK) == 0);
    rc = esdm_backend_mero_rdwr(eb, obj_handle, start, count,
                                data, M0_CLOVIS_OC_WRITE);
    return rc;
}

int esdm_backend_mero_read (struct esdm_backend_generic *eb,
                            void*    obj_handle,
                            uint64_t start,
                            uint64_t count,
                            void*    data)
{
    int rc;
	assert((start & BLOCKMASK) == 0);
	assert(((start + count) & BLOCKMASK) == 0);
    rc = esdm_backend_mero_rdwr(eb, obj_handle, start, count,
                                data, M0_CLOVIS_OC_READ);
    return rc;
}

int esdm_backend_mero_close(struct esdm_backend_generic *eb,
                            void                        *obj_handle)
{
	struct m0_clovis_obj     *obj;
    int                       rc = 0;

	obj = (struct m0_clovis_obj*)obj_handle;
	m0_clovis_obj_fini(obj);
	free(obj);

	return rc;
}

struct esdm_backend mero_esdm_backend = {
    .eb_magic = 0x3333550033335500,
	.eb_name  = "Mero Clovis",
	.eb_id    = 1234,
	.eb_blocksize = BLOCKSIZE,

	.esdm_backend_init = esdm_backend_mero_init,
	.esdm_backend_fini = esdm_backend_mero_fini,

	.esdm_backend_obj_alloc = esdm_backend_mero_alloc,
	.esdm_backend_obj_open  = esdm_backend_mero_open,
	.esdm_backend_obj_write = esdm_backend_mero_write,
	.esdm_backend_obj_read  = esdm_backend_mero_read,
	.esdm_backend_obj_close = esdm_backend_mero_close
};
