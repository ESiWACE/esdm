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

#include<esdm.h>

static struct esdm_backend mero_esdm_backend = {
	eb_magic = 0x3333550033335500,
	eb_name  = "Mero Clovis",
	eb_id    = 1234,
	eb_blocksize = 4096,

	esdm_backend_init = NULL,
	esdm_backend_fini = NULL,

	esdm_backend_obj_alloc = NULL,
	esdm_backend_obj_open  = NULL,
	esdm_backend_obj_write = NULL,
	esdm_backend_obj_read  = NULL,
	esdm_backend_obj_close = NULL
};


#if 0

int esdm_backend_mero_init(char * conf, struct esdm_backend_generic **eb_out)
{
	struct esdm_backend_mero *ebm;
	int rc;

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

	out = &ebm->ebm_base;
	return 0;
}

int esdm_backend_mero_fini(struct esdm_backend_generic *eb)
{
	struct esdm_backend_mero *ebm;

	ebm = container_of(eb, struct esdm_backend_mero, ebm_base);
	m0_clovis_fini(ebm->ebm_clovis_instance, true);
	free(ebm);
	return 0;
}
static void open_entity(struct m0_clovis_obj *obj)
{
	struct m0_clovis_entity *entity = &obj->ob_entity;
	struct m0_clovis_op *ops[1] = { NULL };

	m0_clovis_entity_open(entity, &ops[0]);
	m0_clovis_op_launch(ops, 1);
	m0_clovis_op_wait(ops[0], M0_BITS(M0_CLOVIS_OS_FAILED,
					  M0_CLOVIS_OS_STABLE),
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

	m0_clovis_obj_init(&obj, &ebm->ebm_clovis_container.clovis_uber_realm,
			   &id,
			   m0_clovis_default_layout_id(ebm->ebm_clovis_instance));

	open_entity(&obj);

	m0_clovis_entity_create(&obj.ob_entity, &ops[0]);

	m0_clovis_op_launch(ops, ARRAY_SIZE(ops));

	rc = m0_clovis_op_wait(
		ops[0], M0_BITS(M0_CLOVIS_OS_FAILED, M0_CLOVIS_OS_STABLE),
		m0_time_from_now(3,0));

	m0_clovis_op_fini(ops[0]);
	m0_clovis_op_free(ops[0]);
	//m0_clovis_entity_fini(&obj.ob_entity);
	m0_clovis_obj_fini(&obj);

	return rc;
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

	ebm = container_of(eb, struct esdm_backend_mero, ebm_base);

	/* First step: alloc a new fid for this new object. */
	obj_id = object_id_alloc();

	/* Then create object */
	rc = create_object(ebm, obj_id);
	if (rc == 0) {
		/* encode this obj_id into string */
		*out_object_id = object_id_encode(obj_id);
		*out_mero_metadata = object_meta_encode(obj_id);
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
	int                       rc;

	ebm = container_of(eb, struct esdm_backend_mero, ebm_base);

	/* convert from json string to object id. */
	obj_id = object_id_decode(object_id);

	obj = maolloc(sizeof *obj);
	if (obj == NULL)
		return -ENOMEM;

	memset(obj, 0, sizeof(struct m0_clovis_obj));
	m0_clovis_obj_init(obj, &ebm->ebm_clovis_container.clovis_uber_realm,
			   &obj_id,
			   m0_clovis_default_layout_id(ebm->ebm_clovis_instance));

	open_entity(obj);
	*obj_handle = obj;

	return 0;
}

#define PAGE_MASK (409ULL - 1)

int esdm_backend_mero_write(struct esdm_backend_generic *eb,
			    void*    obj_handle,
			    uint64_t start,
			    int      count,
			    void*    data)
{
	struct m0_clovis_obj* ebm_obj = (struct m0_clovis_obj*)obj_handle;

	/* First check if the start is block-aligned (4K-aligned). */
	if (start & PAGE_MASK) {
		/*
		 * Read the first the page and modify the buffer.
		 * A new buffer is allocated.
		 */
	}
	if ((start + count) & PAGE_MASK) {
		/*
		 * Read the last page and modify the buffer.
		 * A new buffer is allocated.
		 */
	}
	/*
	 * Write these buffers.
	 */
}

int esdm_backend_mero_read (struct esdm_backend_generic *eb,
			    void*    obj_handle,
			    uint64_t start,
			    int      count,
			    void*    data)
{
	struct m0_clovis_obj* ebm_obj = (struct m0_clovis_obj*)obj_handle;

	/* First check if the start is block-aligned (4K-aligned). */
	if (start & PAGE_MASK) {
		/*
		 * extend the 'start'. A new buffer is allocated to
		 * cover it.
		 */
	}
	if ((start + count) & PAGE_MASK) {
		/*
		 * extend the 'count'. A new buffer is allocated to
		 * cover it.
		 */
	}
	/*
	 * read the extended region and copy the data from new buffers.
	 */
}

int esdm_backend_mero_close(struct esdm_backend_generic *eb,
			    void*    obj_handle)
{
	struct esdm_backend_mero *ebm;
	struct m0_clovis_obj     *obj;

	ebm = container_of(eb, struct esdm_backend_mero, ebm_base);

	obj = (struct m0_clovis_obj*)obj_handle;
	m0_clovis_obj_fini(obj);
	free(obj);

	return 0;
}


#endif
