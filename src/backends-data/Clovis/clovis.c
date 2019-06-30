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

/**
 * @file
 * @brief A data backend to provide Clovis compatibility.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef NDEBUG

#include <esdm-debug.h>
#include <esdm.h>

#include "clovis.h"
#include "clovis_internal.h"

#define DEBUG(fmt) ESDM_DEBUG(fmt)
#define DEBUG_FMT(fmt, ...) ESDM_DEBUG_COM_FMT("CLOVIS", fmt, __VA_ARGS__)

#define PAGE_4K (4096ULL)
#define BLOCKSIZE (PAGE_4K)
#define BLOCKMASK (BLOCKSIZE - 1)
#define CLOVIS_OBJ_ID "obj_id"


static struct m0_uint128 gid;
static struct m0_fid gidxfid = {
.f_container = 0x1ULL,
.f_key       = 0x1234567812345678ULL};


int clovis_index_create(struct m0_clovis_realm *parent,
struct m0_fid *fid);


static inline esdm_backend_t_clovis_t *eb2ebm(esdm_backend_t *eb) {
  return container_of(eb, esdm_backend_t_clovis_t, ebm_base);
}

/**
 * @TODO: Please note, if multiple apps are running from the same node,
 * different local addresses are required.
 *  */
char *laddr_get() {
  FILE *output;
  char screen[4096];
  char *first_newline;
  int rc;

  output = popen("/mero.bin/bin/m0nettest -l | grep NID | egrep -e 'tcp|o2ib' | head -n 1 | awk -e '{print $3}'", "r");
  if (output == NULL) {
    return NULL;
  }

  memset(screen, 0, 4096);
  rc = fread(screen, 1, 4096, output);
  pclose(output);
  if (rc > 0) {
    /* find the first '\n' char and replace it with '\0' */
    first_newline = strchr(screen, '\n');
    if (first_newline != 0)
      *first_newline = '\0';
    DEBUG_FMT("local addr = %s", screen);
  }
  return rc > 0 ? strdup(screen) : NULL;
}

/**
 * @TODO Use the laddr_get() to generate a unique local address.
 * This is needed for running in MPI with the same configuration file,
 * and actually every client needs its own local address.
 */
static int conf_parse(char *conf, esdm_backend_t_clovis_t *ebm) {
  /*
     * Parse the conf string into ebm->ebm_clovis_conf.
     */
  char *clovis_local_addr;
  char *clovis_ha_addr;
  char *clovis_prof;
  char *clovis_proc_fid;
  char *clovis_index_dir            = "/tmp/";
  struct m0_idx_dix_config dix_conf = {.kc_create_meta = false};
  /*char *laddr, *combined_laddr;*/

  if ((clovis_local_addr = strsep(&conf, " ")) == NULL)
    return -EINVAL;
#if 0
    /* Temporarily disabled this. */
    laddr = laddr_get();
    if (laddr == NULL)
        return -EINVAL;
    /* concatenate the local address and user provided appendix */
    asprintf(&combined_laddr, "%s%s", laddr, clovis_local_addr);
    free(laddr);
    if (combined_laddr == NULL)
        return -EINVAL;
    ebm->ebm_clovis_conf.cc_local_addr = combined_laddr;
#else
  ebm->ebm_clovis_conf.cc_local_addr = clovis_local_addr;
#endif

  if ((clovis_ha_addr = strsep(&conf, " ")) == NULL)
    return -EINVAL;
  ebm->ebm_clovis_conf.cc_ha_addr = strdup(clovis_ha_addr);

  if ((clovis_prof = strsep(&conf, " ")) == NULL)
    return -EINVAL;
  ebm->ebm_clovis_conf.cc_profile = strdup(clovis_prof);

  if ((clovis_proc_fid = strsep(&conf, " ")) == NULL)
    return -EINVAL;
  ebm->ebm_clovis_conf.cc_process_fid = strdup(clovis_proc_fid);

  ebm->ebm_clovis_conf.cc_is_oostore            = true;
  ebm->ebm_clovis_conf.cc_is_read_verify        = false;
  ebm->ebm_clovis_conf.cc_tm_recv_queue_min_len = M0_NET_TM_RECV_QUEUE_DEF_LEN;
  ebm->ebm_clovis_conf.cc_max_rpc_msg_size      = M0_RPC_DEF_MAX_RPC_MSG_SIZE;
  ebm->ebm_clovis_conf.cc_layout_id             = 0;
  /* for DIX index type. */
  ebm->ebm_clovis_conf.cc_idx_service_id   = M0_CLOVIS_IDX_DIX;
  ebm->ebm_clovis_conf.cc_idx_service_conf = &dix_conf;
  /* for MOCK index type. Use DIX or MOCK. */
  ebm->ebm_clovis_conf.cc_idx_service_id   = M0_CLOVIS_IDX_MOCK;
  ebm->ebm_clovis_conf.cc_idx_service_conf = clovis_index_dir;

  DEBUG_FMT("local addr = %s", ebm->ebm_clovis_conf.cc_local_addr);
  DEBUG_FMT("ha addr    = %s", ebm->ebm_clovis_conf.cc_ha_addr);
  DEBUG_FMT("profile    = %s", ebm->ebm_clovis_conf.cc_profile);
  DEBUG_FMT("process id = %s", ebm->ebm_clovis_conf.cc_process_fid);
  return 0;
}


static int esdm_backend_t_clovis_init(char *conf, esdm_backend_t *eb) {
  esdm_backend_t_clovis_t *ebm;
  time_t t;
  unsigned int pid;
  unsigned int r;
  unsigned long long f;
  int rc;

  ebm = eb2ebm(eb);

  rc = conf_parse(conf, ebm);
  if (rc != 0) {
    return rc;
  }

  /* Clovis instance */
  rc = m0_clovis_init(&ebm->ebm_clovis_instance, &ebm->ebm_clovis_conf, true);
  if (rc != 0) {
    DEBUG_FMT("Failed to initilise Clovis: %d", rc);
    return rc;
  }

  /* And finally, clovis root realm */
  m0_clovis_container_init(&ebm->ebm_clovis_container,
  NULL, &M0_CLOVIS_UBER_REALM,
  ebm->ebm_clovis_instance);
  rc = ebm->ebm_clovis_container.co_realm.re_entity.en_sm.sm_rc;

  if (rc != 0) {
    DEBUG("Failed to open uber realm");
    return rc;
  }

  /* FIXME this makes the gid not reused. */
  t = time(NULL);
  srand(t);
  r        = rand();
  pid      = getpid();
  f        = (t << 16) | (r & 0xff00) | (pid & 0xff);
  gid.u_hi = f;
  gid.u_lo = 1L;
  DEBUG_FMT("GID set to: <%lx:%lx>", gid.u_hi, gid.u_lo);

  /* create the global mapping index */
  /* XXX NO NEED TO DO SO.
    rc = clovis_index_create(&ebm->ebm_clovis_container.co_realm,
                             &gidxfid);
    */

  return rc;
}


static int esdm_backend_t_clovis_fini(esdm_backend_t *eb) {
  esdm_backend_t_clovis_t *ebm;

  ebm = eb2ebm(eb);
  m0_clovis_fini(ebm->ebm_clovis_instance, true);
  free((char *)ebm->ebm_clovis_conf.cc_local_addr);
  free((char *)ebm->ebm_clovis_conf.cc_ha_addr);
  free((char *)ebm->ebm_clovis_conf.cc_profile);
  free((char *)ebm->ebm_clovis_conf.cc_process_fid);
  return 0;
}


static void open_entity(struct m0_clovis_obj *obj) {
  struct m0_clovis_entity *entity = &obj->ob_entity;
  struct m0_clovis_op *ops[1]     = {NULL};

  m0_clovis_entity_open(entity, &ops[0]);
  m0_clovis_op_launch(ops, 1);
  m0_clovis_op_wait(ops[0], M0_BITS(M0_CLOVIS_OS_FAILED, M0_CLOVIS_OS_STABLE),
  M0_TIME_NEVER); //m0_time_from_now(3,0));
  m0_clovis_op_fini(ops[0]);
  m0_clovis_op_free(ops[0]);
  ops[0] = NULL;
}


static int create_object(esdm_backend_t_clovis_t *ebm,
struct m0_uint128 id) {
  int rc = 0;
  struct m0_clovis_obj obj;
  struct m0_clovis_op *ops[1] = {NULL};

  memset(&obj, 0, sizeof(struct m0_clovis_obj));

  m0_clovis_obj_init(&obj, &ebm->ebm_clovis_container.co_realm,
  &id,
  m0_clovis_layout_id(ebm->ebm_clovis_instance));

  open_entity(&obj);

  m0_clovis_entity_create(NULL, &obj.ob_entity, &ops[0]);

  m0_clovis_op_launch(ops, ARRAY_SIZE(ops));

  rc = m0_clovis_op_wait(ops[0],
  M0_BITS(M0_CLOVIS_OS_FAILED, M0_CLOVIS_OS_STABLE),
  M0_TIME_NEVER); //m0_time_from_now(3,0));

  m0_clovis_op_fini(ops[0]);
  m0_clovis_op_free(ops[0]);
  m0_clovis_obj_fini(&obj);

  return rc;
}


static struct m0_uint128 object_id_alloc() {
  /* gid.u_hi keeps unchanged in a one session. */
  gid.u_lo++;
  return gid;
}


static char *object_id_encode(const struct m0_uint128 *obj_id) {
  /* "oid=<0x1234567812345678:0x1234567812345678>" */
  char *json = NULL;
  struct m0_fid fid;

  fid.f_container = obj_id->u_hi;
  fid.f_key       = obj_id->u_lo;

  asprintf(&json, "oid=" FID_F, FID_P(&fid));
  return json;
}


static int object_id_decode(char *oid_json, struct m0_uint128 *obj_id) {
  struct m0_fid fid;
  int rc;
  char *oid = strchr(oid_json, '=');

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


static char *object_meta_encode(const struct m0_uint128 *obj_id) {
  return NULL;
}


static int esdm_backend_t_clovis_alloc(esdm_backend_t *eb,
int n_dims,
int *dims_size,
esdm_type type,
char *md1,
char *md2,
char **out_object_id,
char **out_mero_metadata) {
  esdm_backend_t_clovis_t *ebm;
  struct m0_uint128 obj_id;
  int rc;

  ebm = eb2ebm(eb);

  /* First step: alloc a new fid for this new object. */
  obj_id = object_id_alloc();
  DEBUG_FMT("new obj id = <%lx:%lx>", FID_P((struct m0_fid *)&obj_id));

  /* Then create object */
  rc = create_object(ebm, obj_id);
  if (rc == 0) {
    /* encode this obj_id into string */
    *out_object_id     = object_id_encode(&obj_id);
    *out_mero_metadata = object_meta_encode(&obj_id);
  }
  return rc;
}


static int esdm_backend_t_clovis_open(esdm_backend_t *eb,
char *object_id,
void **obj_handle) {
  esdm_backend_t_clovis_t *ebm;
  struct m0_clovis_obj *obj;
  struct m0_uint128 obj_id;
  int rc = 0;

  ebm = eb2ebm(eb);

  /* convert from json string to object id. */
  object_id_decode(object_id, &obj_id);

  obj = malloc(sizeof *obj);
  if (obj == NULL)
    return -ENOMEM;

  memset(obj, 0, sizeof(struct m0_clovis_obj));
  m0_clovis_obj_init(obj, &ebm->ebm_clovis_container.co_realm,
  &obj_id,
  m0_clovis_layout_id(ebm->ebm_clovis_instance));

  open_entity(obj);
  *obj_handle = obj;

  return rc;
}


static int esdm_backend_t_clovis_rdwr(esdm_backend_t *eb,
void *obj_handle,
uint64_t start,
uint64_t count,
void *data,
int rdwr_op) {
  struct m0_clovis_obj *obj = (struct m0_clovis_obj *)obj_handle;
  uint64_t i;
  struct m0_clovis_op *ops[1] = {NULL};
  struct m0_indexvec ext;
  struct m0_bufvec data_buf;
  struct m0_bufvec attr_buf;
  uint64_t clovis_block_count;
  uint64_t clovis_block_size;
  int rc;

  assert((start & BLOCKMASK) == 0);
  assert(((start + count) & BLOCKMASK) == 0);
  assert(rdwr_op == M0_CLOVIS_OC_READ || rdwr_op == M0_CLOVIS_OC_WRITE);

  /*
     * read the extended region and copy the data from new buffers.
     */

  clovis_block_size  = BLOCKSIZE;
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
  if (rc != 0) {
    m0_indexvec_free(&ext);
    m0_bufvec_free2(&data_buf);
    return rc;
  }

  for (i = 0; i < clovis_block_count; i++) {
    ext.iv_index[i]       = start + clovis_block_size * i;
    ext.iv_vec.v_count[i] = clovis_block_size;

    data_buf.ov_buf[i]         = data + clovis_block_size * i;
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


static int esdm_backend_t_clovis_write(esdm_backend_t *eb,
void *obj_handle,
uint64_t start,
uint64_t count,
void *data) {
  int rc;
  assert((start & BLOCKMASK) == 0);
  assert(((start + count) & BLOCKMASK) == 0);
  rc = esdm_backend_t_clovis_rdwr(eb, obj_handle, start, count,
  data, M0_CLOVIS_OC_WRITE);
  return rc;
}


static int esdm_backend_t_clovis_read(esdm_backend_t *eb,
void *obj_handle,
uint64_t start,
uint64_t count,
void *data) {
  int rc;
  assert((start & BLOCKMASK) == 0);
  assert(((start + count) & BLOCKMASK) == 0);
  rc = esdm_backend_t_clovis_rdwr(eb, obj_handle, start, count,
  data, M0_CLOVIS_OC_READ);
  return rc;
}


static int esdm_backend_t_clovis_close(esdm_backend_t *eb,
void *obj_handle) {
  struct m0_clovis_obj *obj;
  int rc = 0;

  obj = (struct m0_clovis_obj *)obj_handle;
  m0_clovis_obj_fini(obj);
  free(obj);

  return rc;
}


static int esdm_backend_t_clovis_performance_estimate() {
  return 0;
}


static int index_op_tail(struct m0_clovis_entity *ce,
struct m0_clovis_op *op,
int rc) {
  if (rc == 0) {
    m0_clovis_op_launch(&op, 1);
    rc = m0_clovis_op_wait(op,
    M0_BITS(M0_CLOVIS_OS_FAILED,
    M0_CLOVIS_OS_STABLE),
    M0_TIME_NEVER);
    DEBUG_FMT("operation (%d) rc: %i", op->op_code, op->op_rc);
  } else
    DEBUG_FMT("operation (%d) fail rc: %i", op->op_code, rc);
  m0_clovis_op_fini(op);
  m0_clovis_op_free(op);
  m0_clovis_entity_fini(ce);
  return rc;
}


int clovis_index_create(struct m0_clovis_realm *parent,
struct m0_fid *fid) {
  struct m0_clovis_op *op = NULL;
  struct m0_clovis_idx idx;
  int rc = 0;

  m0_clovis_idx_init(&idx, parent, (struct m0_uint128 *)fid);
  rc = m0_clovis_entity_create(NULL, &idx.in_entity, &op);
  rc = index_op_tail(&idx.in_entity, op, rc);
  return rc;
}


static int index_op(struct m0_clovis_realm *parent,
struct m0_fid *fid,
enum m0_clovis_idx_opcode opcode,
struct m0_bufvec *keys,
struct m0_bufvec *vals) {
  struct m0_clovis_idx idx;
  struct m0_clovis_op *op = NULL;
  int32_t *rcs;
  int rc;

  M0_ASSERT(keys != NULL);
  M0_ASSERT(keys->ov_vec.v_nr != 0);
  M0_ALLOC_ARR(rcs, keys->ov_vec.v_nr);
  if (rcs == NULL)
    return -ENOMEM;
  m0_clovis_idx_init(&idx, parent, (struct m0_uint128 *)fid);
  rc = m0_clovis_idx_op(&idx, opcode, keys, vals, rcs, 0, &op);
  rc = index_op_tail(&idx.in_entity, op, rc);
  m0_free(rcs);
  return rc;
}


static int clovis_index_put(struct m0_clovis_realm *parent,
struct m0_fid *fid,
struct m0_bufvec *keys,
struct m0_bufvec *vals) {
  int rc = 0;

  M0_PRE(fid != NULL);
  M0_PRE(keys != NULL);
  M0_PRE(vals != NULL);

  rc = index_op(parent, fid, M0_CLOVIS_IC_PUT, keys, vals);
  DEBUG_FMT("put done: %i", rc);

  return rc;
}


static int clovis_index_get(struct m0_clovis_realm *parent,
struct m0_fid *fid,
struct m0_bufvec *keys,
struct m0_bufvec *vals) {
  int rc;

  M0_PRE(fid != NULL);
  M0_PRE(keys != NULL && keys->ov_vec.v_nr == 1);
  M0_PRE(vals != NULL && vals->ov_vec.v_nr == 1);

  rc = index_op(parent, fid, M0_CLOVIS_IC_GET, keys, vals);
  if (rc == 0) {
    if (vals->ov_buf[0] == NULL)
      rc = -ENODATA;
  }
  DEBUG_FMT("get done: %i", rc);
  return rc;
}


static int bufvec_fill(const char *value, struct m0_bufvec *vals) {
  int rc;

  rc = m0_bufvec_empty_alloc(vals, 1);
  if (rc < 0)
    return rc;

  if (value != NULL) {
    vals->ov_buf[0]         = strdup(value);
    vals->ov_vec.v_count[0] = strlen(value);
  }

  return rc;
}


static int mapping_get(esdm_backend_t *backend,
const char *name,
char **obj_id) {
  esdm_backend_t_clovis_t *ebm = eb2ebm(backend);
  int rc;
  struct m0_bufvec key;
  struct m0_bufvec val;

  rc = bufvec_fill(name, &key) ?: bufvec_fill(NULL, &val);

  if (rc == 0) {
    rc = clovis_index_get(&ebm->ebm_clovis_container.co_realm,
    &gidxfid,
    &key,
    &val);
    if (rc == 0) {
      /* malloc & copy & setting trailing zero. */
      int datalen = val.ov_vec.v_count[0];
      char *res   = malloc(datalen + 1);
      if (res != NULL) {
        memcpy(res, val.ov_buf[0], datalen);
        res[datalen] = 0;
        *obj_id      = res;
      } else
        rc = -ENOMEM;
    }
  }

  m0_bufvec_free(&key);
  m0_bufvec_free(&val);

  return rc;
}


static int mapping_insert(esdm_backend_t *backend,
const char *name,
const char *obj_id) {
  esdm_backend_t_clovis_t *ebm = eb2ebm(backend);
  int rc;
  struct m0_bufvec key;
  struct m0_bufvec val;

  rc = bufvec_fill(name, &key) ?: bufvec_fill(obj_id, &val);

  if (rc == 0) {
    rc = clovis_index_put(&ebm->ebm_clovis_container.co_realm,
    &gidxfid,
    &key,
    &val);
  }

  m0_bufvec_free(&key);
  m0_bufvec_free(&val);

  return rc;
}


static int esdm_backend_t_clovis_fragment_retrieve(esdm_backend_t *backend,
esdm_fragment_t *fragment,
json_t *metadata) {
  char *obj_id     = NULL;
  void *obj_handle = NULL;
  int rc           = 0;
  const char *key;
  json_t *value;

  if (!backend || !fragment || !fragment->buf || !metadata)
    return ESDM_ERROR;

  /*
     * Parse metadata json to retrieve object id.
     */
  json_object_foreach(metadata, key, value) {
    if (!strcmp(key, CLOVIS_OBJ_ID)) {
      obj_id = strdup(json_string_value(value));
      break;
    }
  }

  if (obj_id == NULL) {
    DEBUG("INVALID metadata. No object ID is found");
    return ESDM_ERROR;
  }

  DEBUG_FMT("Retrieving from %s", obj_id);

  // 2. open object with its object_id.
  rc = esdm_backend_t_clovis_open(backend, obj_id, &obj_handle);
  if (rc == 0) {
    // 3. read from this object.
    rc = esdm_backend_t_clovis_read(backend, obj_handle, 0, fragment->bytes, fragment->buf);

    // 4. close this object.
    esdm_backend_t_clovis_close(backend, obj_handle);
  }

  free(obj_id);
  return rc == 0 ? ESDM_SUCCESS : ESDM_ERROR;
}


static int esdm_backend_t_clovis_fragment_update(esdm_backend_t *backend,
esdm_fragment_t *fragment) {
  char *obj_id     = NULL;
  char *obj_meta   = NULL;
  void *obj_handle = NULL;
  int rc           = ESDM_SUCCESS;

  // 1. create a new object for this fragment
  rc = esdm_backend_t_clovis_alloc(backend,
  fragment->dataspace->dims,
  NULL, //fragment->dataspace->size,
  0,
  NULL,
  NULL,
  &obj_id,
  &obj_meta);
  if (rc != 0)
    goto err;

  DEBUG_FMT("Updating to %s (size=%lu)", obj_id, fragment->bytes);
  // 2. open object with its object_id.
  rc = esdm_backend_t_clovis_open(backend, obj_id, &obj_handle);
  if (rc == 0) {
    // 3. write to this object.
    rc = esdm_backend_t_clovis_write(backend, obj_handle, 0,
    fragment->bytes, fragment->buf);

    // 4. close this object.
    esdm_backend_t_clovis_close(backend, obj_handle);
  }

  /*
     * Store this object's ID into fragment metadata. This metadata will be
     * used later in retrieve() to read data from.
     */
  fragment->metadata->size += sprintf(&fragment->metadata->json[fragment->metadata->size],
  "{\"%s\" : \"%s\"}", CLOVIS_OBJ_ID, obj_id);

err:
  free(obj_id);
  free(obj_meta);
  return rc == 0 ? ESDM_SUCCESS : ESDM_ERROR;
}


static int esdm_backend_t_clovis_mkfs(esdm_backend_t *backend, int enforce_format) {
  if (!backend)
    return ESDM_ERROR;

  return ESDM_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


esdm_backend_t_clovis_t esdm_backend_t_clovis = {
.ebm_base = {
.name      = "CLOVIS",
.type      = SMD_DTYPE_DATA,
.version   = "0.0.1",
.data      = NULL,
.blocksize = BLOCKSIZE,
.callbacks = {
(int (*)())esdm_backend_t_clovis_fini,      // finalize
esdm_backend_t_clovis_performance_estimate, // performance_estimate

(int (*)())esdm_backend_t_clovis_alloc,
(int (*)())esdm_backend_t_clovis_open,
(int (*)())esdm_backend_t_clovis_write,
(int (*)())esdm_backend_t_clovis_read,
(int (*)())esdm_backend_t_clovis_close,

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

NULL,                                               // fragment create
(int (*)())esdm_backend_t_clovis_fragment_retrieve, // fragment retrieve
(int (*)())esdm_backend_t_clovis_fragment_update,   // fragment update
NULL,                                               // fragment delete
(int (*)())esdm_backend_t_clovis_mkfs,              // mkfs
},
},
.ebm_ops = {
.esdm_backend_t_init = esdm_backend_t_clovis_init,
.esdm_backend_t_fini = esdm_backend_t_clovis_fini,

.esdm_backend_t_obj_alloc = esdm_backend_t_clovis_alloc,
.esdm_backend_t_obj_open  = esdm_backend_t_clovis_open,
.esdm_backend_t_obj_write = esdm_backend_t_clovis_write,
.esdm_backend_t_obj_read  = esdm_backend_t_clovis_read,
.esdm_backend_t_obj_close = esdm_backend_t_clovis_close,
.mapping_get              = mapping_get,
.mapping_insert           = mapping_insert,
},
};


esdm_backend_t *clovis_backend_init(esdm_config_backend_t *config) {
  esdm_backend_t *eb = &esdm_backend_t_clovis.ebm_base;
  char *target       = NULL;
  int rc;

  if (!config || !config->type || strcasecmp(config->type, "CLOVIS") || !config->target) {
    DEBUG("Wrong configuration");
    return NULL;
  }

  DEBUG_FMT("backend type   = %s", config->type);
  DEBUG_FMT("backend id     = %s", config->id);
  DEBUG_FMT("backend target = %s", config->target);

  /*
     *          "local_addr ha_addr profile process_fid"
     * target = ":12345:33:103 172.16.154.130@tcp:12345:34:1 <0x7000000000000001:0> <0x7200000000000001:64>";
     * Now please note the local_addr does not include the ipaddr.
     */
  target = strdup(config->target);
  if (target == NULL)
    return NULL;

  rc = esdm_backend_t_clovis_init(target, eb);
  free(target);

  if (rc != 0)
    return NULL;
  else
    return eb;
}
