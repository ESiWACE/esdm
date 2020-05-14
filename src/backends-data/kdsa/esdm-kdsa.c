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
 */

/**
 * @file
 * @brief A data backend to provide Kove XPD KDSA compatibility.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <esdm-internal.h>


#include <kdsa.h>

#include "esdm-kdsa.h"

#define XPD_FLAGS (KDSA_FLAGS_HANDLE_IO_NOSPIN|KDSA_FLAGS_HANDLE_USE_EVENT)

#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("KDSA", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("KDSA", fmt, __VA_ARGS__)

#define WARN_ENTER ESDM_WARN_COM_FMT("KDSA", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("KDSA", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("KDSA", "%s", fmt)

#define ERROR(fmt, ...) ESDM_ERROR_COM_FMT("KDSA", fmt, __VA_ARGS__)
#define ERRORS(fmt) ESDM_ERROR_COM_FMT("KDSA", "%s", fmt)

#define WARN_STRERR(fmt, ...) WARN(fmt ": %s", __VA_ARGS__, strerror(errno));
#define WARN_CHECK_RET(ret, fmt, ...) if(ret != 0){ WARN(fmt ": %s", __VA_ARGS__, strerror(errno)); }

#define ESDM_MAGIC 69083068077013010ull

typedef kdsa_vol_handle_t handle_t;

/*
 Physical data layout
 HEADER
  ESDM_MAGIC (64 bit)
  blocksize (64 bit) => blocksize in byte; each block will host one fragment
  blockcount (64 bit) => the number of blocks
  start of data blocks => uint64_t to the location of the first data block
 Block map (used blocks in 64 bits each)
  for each block: 0 == unused, 1 == used
  => The block bitmap must be updated consistently by all processes/nodes.
  TODO: could use blocks for large and small data: buddy system?
 Data blocks
  Block0: Data
  Block1: Data
  ...
  BlockN: Data
  Where the number is the block ID.

  A fragment is mapped as B blocks where B-1 blocks are fully filled

 Algorithms:
   * format: overwrite header and block map using kdsa_memset() then write new header
     * compute size for block map based on volume size
   * initialize:
     * compute size for block map based on volume size
     * allocate block map
     * load header and block map data into memory, register block map region
     * consistency check => MAGIC + start of data blocks correct?
   * write:
     * find free blocks asynchronously (this could actually be done in the background as well)
       Assume the block bitmap is valid.
       Check how many blocks are free.
          If fragment size + free space would exceed 90%, retrieve current block map. Change block search to sequential.
       Repeat until all necessary blocks are found:
          Chose a random 64-bit block offset, if there are one or multiple free blocks out of 64 available blocks inside (thus, value is not 2^64-1), reserve the  necessary blocks. If sequential mode, continue search from here. If cannot find a free block after turnaround => write error.
          execute kdsa_async_compare_and_swap() for each
       Check for return.
         if one or multiple blocks value was updated concurrently, we update the value in our block bitmap.
         start IOs asynchronously for blocks that are reserved and we can already start
       Update believed free blocks
       Write asynchronously all blocks out
   * read strategies:
     complete: load the full fragment
     sparse: identify all needed locations, depending on sparsity either run complete strategy OR read all regions directly asynchronously to target memory location
   * store in JSON metadata the block IDs that were used (may use delta compression upon write of MD). This could theoretically be stored as the ID of the fragment allowing to infer from ID directly the blocks it is stored in!
      => the actual size of the fragment is stored as part of the metadata anyway
 */

typedef struct{
  uint64_t magic;
  uint64_t blocksize;
  uint64_t blockcount;
  uint64_t offset_to_data;
} kdsa_persistent_header_t;



// Internal functions used by this backend.
typedef struct {
  esdm_config_backend_t *config;
  handle_t handle;
  kdsa_size_t size;
  esdm_perf_model_lat_thp_t perf_model;

  kdsa_persistent_header_t h;

  pthread_spinlock_t block_lock; // lock for updating the block map
  uint64_t * block_map;
  uint64_t free_blocks_estimate;
} kdsa_backend_data_t;

typedef struct{
  uint64_t offset; // KDSA offset
} kdsa_fragment_metadata_t;


///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint64_t calc_block_map_size(uint64_t blocks){
  return (blocks+63) / 64;
}

static uint64_t calc_block_count(uint64_t blocksize, uint64_t volume_size){
  if(volume_size <= sizeof(kdsa_persistent_header_t)){
    return 0;
  }
  // size = header + blocks/64 + blocks * blocksize
  // => blocks = (size - hdr) / (1/64 + blocksize)
  // multiply with 64 for integer division, round down (automatically)
  uint64_t blocks = 64 * (volume_size - sizeof(kdsa_persistent_header_t)) / (1 + 64*blocksize);
  return blocks;
}

static int load_block_bitmap(kdsa_backend_data_t *data){
  uint64_t blockmap_size = calc_block_map_size(data->h.blockcount);
  int ret = kdsa_read_unregistered(data->handle, sizeof(kdsa_persistent_header_t), data->block_map, blockmap_size* sizeof(uint64_t));
  if(ret != 0){
    WARN_STRERR("%s", "Could not update block bitmap");
    data->free_blocks_estimate = 0;
    return -1;
  }
  uint64_t freeb = 0;
  for(uint64_t i = 0; i < blockmap_size; i++){
    uint64_t val = ~ data->block_map[i];
    for(int b = 0; b < 64; b++){
      if( val & 1 ) freeb++;
      val = val >> 1;
    }
  }
  data->free_blocks_estimate = freeb;

  return 0;
}

static int mkfs(esdm_backend_t *backend, int format_flags) {
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  int ret;
  DEBUG("mkfs: backend->(void*)data->config->target = %s\n", data->config->target);

  const char *tgt = data->config->target;
  if (strlen(tgt) < 6) {
    WARNS("safety, tgt connection string shall be longer than 6 chars");
    return ESDM_ERROR;
  }
  int const ignore_err = format_flags & ESDM_FORMAT_IGNORE_ERRORS;

  uint64_t magic;

  if (format_flags & ESDM_FORMAT_DELETE) {
    printf("[mkfs] Cleaning %s\n", tgt);
    ret = kdsa_read_unregistered(data->handle, 0, & magic, sizeof(magic));
    WARN_CHECK_RET(ret, "[mkfs] Error could not read magic from volume %s", tgt);
    if(magic == ESDM_MAGIC){
      magic = 0;
      ret = kdsa_write_unregistered(data->handle, 0, & magic, sizeof(magic));
      WARN_CHECK_RET(ret, "[mkfs] Error could not remove magic from volume %s", tgt);
    }else if(! ignore_err){
      printf("[mkfs] Error %s is not an ESDM KDSA volume\n", tgt);
      return ESDM_ERROR;
    }
  }

  if(! (format_flags & ESDM_FORMAT_CREATE)){
    return ESDM_SUCCESS;
  }
  ret = kdsa_read_unregistered(data->handle, 0, & magic, sizeof(magic));
  WARN_CHECK_RET(ret, "[mkfs] Error could not check magic on volume %s", tgt);
  if(magic == ESDM_MAGIC){
    if(! ignore_err){
      printf("[mkfs] Error volume %s appears to be an ESDM volume already\n", tgt);
      return ESDM_ERROR;
    }
    printf("[mkfs] WARNING volume %s appears to be an ESDM volume already but will reformat\n", tgt);
  }

  uint64_t blocks = calc_block_count(data->h.blocksize, data->size);
  uint64_t blockmap_size = calc_block_map_size(blocks);
  uint64_t offset_to_data = sizeof(kdsa_persistent_header_t) + blockmap_size * sizeof(uint64_t);

  offset_to_data = (offset_to_data + 63) / 64 * 64; // round to 64 bytes

  // fill data structure
  data->h = (kdsa_persistent_header_t){
    .magic = ESDM_MAGIC,
    .blocksize = data->h.blocksize,
    .blockcount = blocks,
    .offset_to_data = offset_to_data
  };

  ret = kdsa_write_unregistered(data->handle, 0, & data->h, sizeof(kdsa_persistent_header_t));
  printf("[mkfs] Formatting %s (size: %.2f GiB) with %lu blocks (size: %.1f MiB) and a blockmap of %lu entries\n", tgt, data->size /1024.0/1024/1024, blocks, data->h.blocksize / 1024.0/1024,  blockmap_size);
  if (ret != 0) {
    WARN_CHECK_RET(ret, "[mkfs] WARNING could not format volume %s", tgt);
    if(! ignore_err){
      return ESDM_ERROR;
    }
  }

  data->block_map = ea_checked_malloc(blockmap_size* sizeof(uint64_t));
  memset(data->block_map, 0, blockmap_size* sizeof(uint64_t));
  data->free_blocks_estimate = blocks;

  // set the occupied bits of the last uint to address the situation when blocks % 64 != 0
  uint64_t val = 0;
  int occupy_blocks = 64 - (blocks % 64);
  for(int b = 0; b < occupy_blocks; b++){
    val = (val>>1) | (1llu<<63); // always occupy the remaining largest block
  }
  data->block_map[blockmap_size-1] = val;

  ret = kdsa_write_unregistered(data->handle, sizeof(kdsa_persistent_header_t), data->block_map, blockmap_size* sizeof(uint64_t));
  if (ret != 0) {
    WARN_CHECK_RET(ret, "[mkfs] WARNING could not format volume %s", tgt);
    if(! ignore_err){
      return ESDM_ERROR;
    }
  }

  return ESDM_SUCCESS;
}

static int fsck(esdm_backend_t* backend) {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void * fragment_metadata_load(esdm_backend_t * b, esdm_fragment_t *f, json_t *md){
  kdsa_fragment_metadata_t * fragmd = ea_checked_malloc(sizeof(kdsa_fragment_metadata_t));
  eassert(f->id);
  long long unsigned offset;
  sscanf(f->id, "%llu", & offset);
  fragmd->offset = offset;

  return fragmd;
}

static int fragment_metadata_free(esdm_backend_t * b, void * f){
  //kdsa_fragment_metadata_t * fragmd = (kdsa_fragment_metadata_t *) f;
  free(f);
  return 0;
}


static int fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  // set data, options and tgt for convienience
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  int ret = 0;
  kdsa_fragment_metadata_t * fragmd = (kdsa_fragment_metadata_t*) f->backend_md;
  if(f->bytes > data->h.blocksize){
    WARN("Error could not read more data than blocksize (%"PRIu64" > %"PRIu64")", f->bytes,  data->h.blocksize);
    return ESDM_ERROR;
  }
  if(f->dataspace->stride) {
    //the data is not necessarily supposed to be contiguous in memory -> read into separate buffer
    void* readBuffer = ea_checked_malloc(f->bytes);
    ret = kdsa_read_unregistered(data->handle, fragmd->offset, readBuffer, f->bytes);
    DEBUG("read: %lu\n", *(uint64_t*) readBuffer);
    if(ret != 0){
      WARN_STRERR("Error could not read data from volume %s", data->config->target);
      free(readBuffer);
      return ESDM_ERROR;
    }

    //copy to the requested data layout
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(contiguousSpace, readBuffer, f->dataspace, f->buf);
    esdm_dataspace_destroy(contiguousSpace);
    free(readBuffer);
  } else {
    ret = kdsa_read_unregistered(data->handle, fragmd->offset, f->buf, f->bytes);
    DEBUG("read: %lu\n", *(uint64_t*) f->buf);
    if(ret != 0){
      WARN_STRERR("Error could not read data from volume %s", data->config->target);
      return ESDM_ERROR;
    }
  }

  return ESDM_SUCCESS;
}

static uint64_t try_to_use_block(kdsa_backend_data_t* data, uint64_t bitmap_pos){
  int ret = 0;
  for(int b = 0; b < 64; b++){
    uint64_t expected = data->block_map[bitmap_pos];

    if(expected == UINT64_MAX){
      return 0;
    }
    uint64_t val = ~expected;
    if( val & (1lu << b) ){
      uint64_t swap = expected | 1lu << b;
      kdsa_vol_offset_t offset = bitmap_pos*sizeof(uint64_t) + sizeof(kdsa_persistent_header_t);
      ret = kdsa_compare_and_swap(data->handle, offset, expected, swap, & data->block_map[bitmap_pos]);
      //printf("%llu %d expected: %llu swap: %llu got: %llu = bit: %d ret: %d offset: %llu\n", bitmap_pos, rank, expected, swap, data->block_map[bitmap_pos], b, ret, offset);

      if (ret != 0){
        ERRORS("Could not invoke kdsa_compare_and_swap()\n");
      }
      if (data->block_map[bitmap_pos] == expected){
        data->block_map[bitmap_pos] = swap;
        // found a block!
        return (b + 64*bitmap_pos) * data->h.blocksize + data->h.offset_to_data;
      }
    }
    val = val >> 1;
  }
  return 0;
}

static uint64_t find_offset_to_store_fragment(kdsa_backend_data_t* data){
  int ret = 0;
  ret = pthread_spin_lock(& data->block_lock);
  if(data->free_blocks_estimate*100 / data->h.blockcount <= 3){
    ret = load_block_bitmap(data);
    if( ret != 0 || data->free_blocks_estimate == 0){
      // could not load or no more space available
      ret = pthread_spin_unlock(& data->block_lock);
      ESDM_WARN_FMT("KDSA: No free block found (loaded from: %lu)", data->free_blocks_estimate);
      return 0;
    }
  }
  // try 30 times to find a free block
  uint64_t offset = 0;
  uint64_t blockmap_size = calc_block_map_size(data->h.blockcount);
  for(int i = 0; i < 30; i++){
    uint64_t bitmap_pos = rand() % blockmap_size; //FIXME: Don't use [s]rand().
    offset = try_to_use_block(data, bitmap_pos);
    if(offset != 0){
      break;
    }
  }
  if(offset == 0){
    // try a sequential strategy starting at one block, this is inefficient!
    uint64_t bitmap_pos = rand() % blockmap_size; //FIXME: Don't use [s]rand().
    for(int i = 0; i < blockmap_size; i++){
      offset = try_to_use_block(data, (i + bitmap_pos) % blockmap_size);
      if(offset != 0){
        break;
      }
    }
  }
  if(offset != 0){
    data->free_blocks_estimate--;
  }else{
    ESDM_WARN_FMT("KDSA: No free block found (assumed free: %lu)", data->free_blocks_estimate);
  }
  ret = pthread_spin_unlock(& data->block_lock);
  return offset;
}


//TODO this has zero test coverage currently
static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;

  int ret;
  // set data, options and tgt for convienience
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;

  kdsa_fragment_metadata_t * fragmd = (kdsa_fragment_metadata_t*) f->backend_md;
  // lazy assignment of ID
  if(! f->id){
    uint64_t offset = find_offset_to_store_fragment(data);
    if(offset == 0){
      return ESDM_ERROR;
    }
    fragmd = ea_checked_malloc(sizeof(kdsa_fragment_metadata_t));
    f->backend_md = fragmd;
    eassert(f->backend_md);
    fragmd->offset = offset;

    f->id = ea_checked_malloc(22);
    eassert(f->id);
    sprintf(f->id, "%"PRId64, offset);
  }

  if(f->bytes > data->h.blocksize){
    WARN("Error could not write more data than blocksize (%"PRIu64" > %"PRIu64")", f->bytes,  data->h.blocksize);
    return ESDM_ERROR;
  }

  DEBUG("write: %lu\n", *(uint64_t*) f->buf);
  if(f->dataspace->stride) {
    //The fragment appears to have a non-trivial stride. Linearize the fragment's data before writing.
    void* writeBuffer = ea_checked_malloc(f->bytes);
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(f->dataspace, f->buf, contiguousSpace, writeBuffer);
    esdm_dataspace_destroy(contiguousSpace);

    ret = kdsa_write_unregistered(data->handle, fragmd->offset, writeBuffer, f->bytes);
    if(ret != 0){
      WARN_STRERR("Error could not write data from volume %s", data->config->target);
      return ESDM_ERROR;
    }
    free(writeBuffer);
  } else {
    //the fragment is a dense array in C index order, write it without any conversions
    ret = kdsa_write_unregistered(data->handle, fragmd->offset, f->buf, f->bytes);
    if(ret != 0){
      WARN_STRERR("Error could not write data from volume %s", data->config->target);
      return ESDM_ERROR;
    }
  }

  return ESDM_SUCCESS;
}

static int fragment_delete(esdm_backend_t * b, esdm_fragment_t *f){
  kdsa_fragment_metadata_t * fragmd = (kdsa_fragment_metadata_t*) f->backend_md;

  if(! fragmd){
    return ESDM_SUCCESS;
  }
  kdsa_backend_data_t *data = (kdsa_backend_data_t *) b->data;

  uint64_t pos = (fragmd->offset - data->h.offset_to_data) / data->h.blocksize;
  uint64_t bitmap_pos = pos / 64;
  int bit = ~(1llu<<(pos % 64));

  while(true){
    int ret = 1;
    uint64_t expected = data->block_map[bitmap_pos];
    uint64_t swap = expected & bit;
    ret = kdsa_compare_and_swap(data->handle, bitmap_pos*sizeof(uint64_t) + sizeof(kdsa_persistent_header_t), expected, swap, & data->block_map[bitmap_pos]);
    if (ret != 0){
      ERRORS("Could not invoke kdsa_compare_and_swap()\n");
    }
    if (data->block_map[bitmap_pos] == expected){
      data->block_map[bitmap_pos] = swap;
      return ESDM_SUCCESS;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int kdsa_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

static float kdsa_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;
  eassert(backend);

  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

int kdsa_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;
  kdsa_backend_data_t *b = (kdsa_backend_data_t *)backend->data;
  int ret = kdsa_disconnect(b->handle);
  if(ret < 0)
  {
    WARN("Failed to disconnect from XPD: %s (%d)\n", strerror(errno), errno);
  }
  WARN("Free blocks : %"PRIu64"\n", b->free_blocks_estimate);
  free(backend->data);
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
  ///////////////////////////////////////////////////////////////////////////////
  // NOTE: This serves as a template for the posix plugin and is memcopied!    //
  ///////////////////////////////////////////////////////////////////////////////
  .name = "KDSA",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = kdsa_finalize,
    .performance_estimate = kdsa_backend_performance_estimate, // performance_estimate
    .estimate_throughput = kdsa_backend_estimate_throughput,
    .fragment_create = NULL,
    .fragment_retrieve = fragment_retrieve,
    .fragment_update = fragment_update,
    .fragment_delete = fragment_delete,
    .fragment_metadata_create = NULL,
    .fragment_metadata_load = fragment_metadata_load,
    .fragment_metadata_free = fragment_metadata_free,
    .mkfs = mkfs,
    .fsck = fsck,
  },
};


esdm_backend_t *kdsa_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "KDSA") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));
  int ret;
  // allocate memory for backend instance
  backend->data = ea_checked_malloc(sizeof(kdsa_backend_data_t));
  kdsa_backend_data_t *data = (kdsa_backend_data_t *)backend->data;
  ret = pthread_spin_init(& data->block_lock, PTHREAD_PROCESS_PRIVATE);
  eassert(ret == 0);

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  // configure backend instance
  data->config = config;
  json_t *elem;
  elem = json_object_get(config->backend, "target");
  char * tgt = (char*) strdup(json_string_value(elem));
  data->config->target = tgt;

  uint64_t blocksize = config->max_fragment_size;
  if(blocksize < 0){
    blocksize = 0;
    ERROR("Blocksize is not valid on volume %s", tgt);
  }
  DEBUG("Backend config: target=%s\n", tgt);

  ret = kdsa_connect(tgt, XPD_FLAGS, & data->handle);
  if(ret < 0){
    ERROR("Failed to connect to XPD: %s (%d)\n", strerror(errno), errno);
  }
  ret = kdsa_get_volume_size(data->handle, & data->size);
  if(ret < 0){
    ERROR("Failed to retrieve volume size %s (%d)\n", strerror(errno), errno);
  }

  ret = kdsa_read_unregistered(data->handle, 0, & data->h, sizeof(kdsa_persistent_header_t));
  if(ret != 0){
    WARN_STRERR("Error could not read header from volume %s", data->config->target);
    return NULL;
  }
  if(data->h.magic != ESDM_MAGIC || blocksize != data->h.blocksize){
    WARN("It appears the volume is no ESDM volume (or config does not fit), will disable write for now on %s expected blocksize: %lu", tgt, blocksize);
    if(blocksize == 0){
      ERROR("Blocksize is not set, cannot proceed as no KDSA volume is found on %s", tgt);
      return NULL;
    }
    data->h.blocksize = blocksize;
    data->h.blockcount = 0;
    data->h.offset_to_data = 0;
    data->block_map = NULL;
  }else if(data->h.blockcount != calc_block_count(data->h.blocksize, data->size)){
    WARN("Blockcount in header does not match the block count determined when retrieving the volume size, it appears the volume size has been changed on %s. The max_fragment_size expected was: %lu", tgt, data->h.blocksize);
    data->h.blocksize = blocksize;
    data->h.blockcount = 0;
    data->h.offset_to_data = 0;
    data->block_map = NULL;
  }else{
    data->block_map = ea_checked_malloc(calc_block_map_size(data->h.blockcount)* sizeof(uint64_t));
    ret = load_block_bitmap(data);
    if( ret != 0 ){
      ERROR("Could not read block bitmap from %s", tgt);
      return NULL;
    }
  }

  return backend;
}
