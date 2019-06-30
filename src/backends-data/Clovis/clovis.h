/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
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
#ifndef CLOVIS_H
#define CLOVIS_H

#include "esdm.h"


/**
* Initializes the CLOVIS plugin. In particular this involves:
*
*    * Load configuration of this backend
*    * Load and potenitally calibrate performance model
*
*    * Connect with support services e.g. for technical metadata
*    * Setup directory structures used by this CLOVIS specific backend
*
*    * Poopulate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/

esdm_backend_t *clovis_backend_init(esdm_config_backend_t *config);

static inline esdm_backend_t_clovis_t *eb2ebm(esdm_backend_t *eb);

/**
 *  To get local network address, in tcp or o2ib.
 * 'lctl' requires root privilege, but m0nettest runs in normal user.
 * This is used to pick local address. Clovis app requires to choose
 * its own unique local address. If ESDM is running from multiple nodes
 * with the same configuration file, a unique local address should be
 * used for every app on different nodes.
 *
 *  */

char *laddr_get();

/**
 * Parse the conf into various parameters.
 */

static int conf_parse(char *conf, esdm_backend_t_clovis_t *ebm);

static int esdm_backend_t_clovis_init(char *conf, esdm_backend_t *eb);

static int esdm_backend_t_clovis_fini(esdm_backend_t *eb);

static void open_entity(struct m0_clovis_obj *obj);

static int create_object(esdm_backend_t_clovis_t *ebm,
struct m0_uint128 id);

/**
* We need a mechansim to allocate id globally, uniquely.
* We may need to store the last allocated id in store and
* read it on start.
*/

static struct m0_uint128 object_id_alloc();

/**
 * A memory of char array is allocated, and
 * the caller needs to free it after use.
 */

static char *object_id_encode(const struct m0_uint128 *obj_id);

static int object_id_decode(char *oid_json, struct m0_uint128 *obj_id);

static char *object_meta_encode(const struct m0_uint128 *obj_id);

static int esdm_backend_t_clovis_alloc(esdm_backend_t *eb,
int n_dims,
int *dims_size,
esdm_type type,
char *md1,
char *md2,
char **out_object_id,
char **out_mero_metadata);

static int esdm_backend_t_clovis_open(esdm_backend_t *eb,
char *object_id,
void **obj_handle);

static int esdm_backend_t_clovis_rdwr(esdm_backend_t *eb,
void *obj_handle,
uint64_t start,
uint64_t count,
void *data,
int rdwr_op);

static int esdm_backend_t_clovis_write(esdm_backend_t *eb,
void *obj_handle,
uint64_t start,
uint64_t count,
void *data);

static int esdm_backend_t_clovis_read(esdm_backend_t *eb,
void *obj_handle,
uint64_t start,
uint64_t count,
void *data);

static int esdm_backend_t_clovis_close(esdm_backend_t *eb,
void *obj_handle);

static int esdm_backend_t_clovis_performance_estimate();

int clovis_index_create(struct m0_clovis_realm *parent,
struct m0_fid *fid);

static int index_op(struct m0_clovis_realm *parent,
struct m0_fid *fid,
enum m0_clovis_idx_opcode opcode,
struct m0_bufvec *keys,
struct m0_bufvec *vals);

static int clovis_index_put(struct m0_clovis_realm *parent,
struct m0_fid *fid,
struct m0_bufvec *keys,
struct m0_bufvec *vals);

static int clovis_index_get(struct m0_clovis_realm *parent,
struct m0_fid *fid,
struct m0_bufvec *keys,
struct m0_bufvec *vals);

static int bufvec_fill(const char *value, struct m0_bufvec *vals);

/**
 * Get the obj_id from mapping DB if this mapping exists.
 *
 * @param obj_id is allocated inside, and should be freed by caller.
 * @return 0 on success, -1 on failure.
 */

static int mapping_get(esdm_backend_t *backend,
const char *name,
char **obj_id);

/**
* Insert this mapping into mapping DB.
* @return 0 on success, -1 on failure.
*/

static int mapping_insert(esdm_backend_t *backend,
const char *name,
const char *obj_id);

static int esdm_backend_t_clovis_fragment_retrieve(esdm_backend_t *backend,
esdm_fragment_t *fragment,
json_t *metadata);

static int esdm_backend_t_clovis_fragment_update(esdm_backend_t *backend,
esdm_fragment_t *fragment);

static int esdm_backend_t_clovis_mkfs(esdm_backend_t *backend, int enforce_format);

#endif
