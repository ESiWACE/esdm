/* This file is part of ESDM. See the enclosed LICENSE */

/**
 * @file
 * @brief Entry point for ESDM streaming implementation
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esdm-internal.h>
#include <esdm-stream.h>

#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("STREAM", fmt, __VA_ARGS__)
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("STREAM", fmt, __VA_ARGS__)


#ifdef HAVE_SCIL
SCIL_Datatype_t ea_esdm_datatype_to_scil(smd_basic_type_t type){
  switch (type) {
    case (SMD_TYPE_UNKNOWN):
      return SCIL_TYPE_UNKNOWN;
    case (SMD_TYPE_UINT8):
    case (SMD_TYPE_INT8):
      return SCIL_TYPE_INT8;
    case (SMD_TYPE_UINT16):
    case (SMD_TYPE_INT16):
      return SCIL_TYPE_INT16;
    case (SMD_TYPE_UINT32):
    case (SMD_TYPE_INT32):
      return SCIL_TYPE_INT32;
    case (SMD_TYPE_UINT64):
    case (SMD_TYPE_INT64):
      return SCIL_TYPE_INT64;
    case (SMD_TYPE_FLOAT):
      return SCIL_TYPE_FLOAT;
    case (SMD_TYPE_DOUBLE):
      return SCIL_TYPE_DOUBLE;
    case (SMD_TYPE_STRING):
      return SCIL_TYPE_STRING;
    default:
      printf("Unsupported datatype: %d\n", type);
      return 0;
  }
}
#endif

bool estream_mem_unpack_fragment_param(esdm_fragment_t *f, void ** out_buf, size_t * out_size){
  if(f->actual_bytes != -1){
    *out_size = f->actual_bytes;
    *out_buf = ea_checked_malloc(f->actual_bytes);
    return TRUE;
  }else{
    *out_size = f->bytes;
  }
  if(f->dataspace->stride) {
    *out_buf = ea_checked_malloc(f->bytes);
    return TRUE;
  }
  *out_buf = f->buf;
  return FALSE;
}

int estream_mem_unpack_fragment(esdm_fragment_t *f, void * rbuff, size_t size){
  if(f->actual_bytes != -1){
    // need to decompress
#ifdef HAVE_SCIL
    SCIL_Datatype_t scil_t = ea_esdm_datatype_to_scil(f->dataspace->type->type);
    scil_dims_t scil_dims;
    scil_dims_initialize_array(& scil_dims, f->dataspace->dims, (size_t*) f->dataspace->size);

    size_t buf_size = scil_get_compressed_data_size_limit(& scil_dims, scil_t);
    byte * scil_buf = ea_checked_malloc(buf_size);
    int ret;

    if(f->dataspace->stride){
      ret = scil_decompress(scil_t, scil_buf, & scil_dims, rbuff, size, scil_buf);
      free(rbuff);
      if (ret != SCIL_NO_ERR){
        free(scil_buf);
        return ESDM_ERROR;
      }
      rbuff = scil_buf;
    }else{
      ret = scil_decompress(scil_t, f->buf, & scil_dims, rbuff, size, scil_buf);
      free(scil_buf);
      free(rbuff);
      return (ret == SCIL_NO_ERR) ? ESDM_SUCCESS : ESDM_ERROR;
    }
#else
    ESDM_WARN("Use ESDM trying to decompress but compiled without SCIL support.");
    return ESDM_ERROR;
#endif
  }

  if(f->dataspace->stride) {
    //data is not necessarily supposed to be contiguous in memory
    // -> copy from contiguous dataspace
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(contiguousSpace, rbuff, f->dataspace, f->buf);
    esdm_dataspace_destroy(contiguousSpace);
    free(rbuff);
  }
  return ESDM_SUCCESS;
}


int estream_mem_pack_fragment(esdm_fragment_t *f, void ** in_out_buff, size_t * out_size){
  int last_phase = 0;

  if(f->dataspace->stride){
    last_phase = 1;
  }

#ifdef HAVE_SCIL
  if(f->dataset->chints && f->dataspace->dims <= 5){
    last_phase = 2;
  }
#endif

  if(last_phase == 0){
    *out_size = f->bytes;
    if(*in_out_buff != NULL){
      // need to copy data to the expected output buffer
      memcpy(*in_out_buff, f->buf, f->bytes);
    }else{
      *in_out_buff = f->buf;
    }
    return ESDM_SUCCESS;
  }

  void* allocBuff = NULL;  // this buffer must be freed
  void* inBuff = f->buf;   // input buffer
  void* outBuff = NULL;    // output buffer
  size_t bytes = f->bytes;

  // phase 1: serialization in memory
  if(f->dataspace->stride){
    //data is not necessarily contiguous in memory -> copy to contiguous dataspace
    if(*in_out_buff != NULL && last_phase == 1){
      outBuff = *in_out_buff; // output buffer
    }else{
      outBuff = ea_checked_malloc(f->bytes);
      allocBuff = outBuff;
    }
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(f->dataspace, inBuff, contiguousSpace, outBuff);
    esdm_dataspace_destroy(contiguousSpace);

    inBuff = outBuff;
    outBuff = NULL;
  }

  // phase 2: compression
  if(f->dataset->chints && f->dataspace->dims <= 5){
#ifdef HAVE_SCIL
    scil_context_t *ctx;
    // TODO handle special values...  int special_values_count, scil_value_t *special_values
    SCIL_Datatype_t scil_t = ea_esdm_datatype_to_scil(f->dataspace->type->type);
    scil_dims_t scil_dims;
    scil_dims_initialize_array(& scil_dims, f->dataspace->dims, (size_t*) f->dataspace->size);

    int ret = scil_context_create(& ctx, scil_t, 0, NULL, f->dataset->chints);
    if(ret != SCIL_NO_ERR) return ESDM_ERROR;

    size_t buf_size = scil_get_compressed_data_size_limit(& scil_dims, scil_t);

    if(*in_out_buff != NULL && last_phase == 2){
      outBuff = *in_out_buff; // output buffer
    }else{
      outBuff = ea_checked_malloc(buf_size);
    }
    size_t out_size = 0;
    ret = scil_compress((byte*)outBuff, buf_size, inBuff, & scil_dims, & out_size, ctx);
    if(ret != SCIL_NO_ERR) return ESDM_ERROR;
    ret = scil_destroy_context(ctx);
    DEBUG("SCIL compressed: %ld => %ld\n", f->bytes, out_size);
    //if(out_size < f->bytes){
    f->actual_bytes = out_size;
    // update data to write
    bytes = out_size;
    if(*in_out_buff == NULL || last_phase != 2){
      if(allocBuff) free(allocBuff);
      allocBuff = outBuff;
    }

    inBuff = outBuff;
    outBuff = NULL;
#endif
  }

  *out_size = bytes;
  if(*in_out_buff == NULL){
    *in_out_buff = inBuff;
  }
  assert(*in_out_buff == inBuff);

  return ESDM_SUCCESS;
}


/*
#ifdef HAVE_SCIL
    scil_context_t *ctx;
    // TODO handle special values...  int special_values_count, scil_value_t *special_values
    SCIL_Datatype_t scil_t = ea_esdm_datatype_to_scil(f->dataspace->type->type);
    scil_dims_t scil_dims;
    scil_dims_initialize_array(& scil_dims, f->dataspace->dims, (size_t*) f->dataspace->size);

    int ret = scil_context_create(& ctx, scil_t, 0, NULL, f->dataset->chints);

    size_t buf_size = scil_get_compressed_data_size_limit(& scil_dims, scil_t);
    scil_buf = ea_checked_malloc(buf_size);
    size_t out_size = 0;

    ret = scil_compress((byte*)scil_buf, buf_size, writeBuffer, & scil_dims, & out_size, ctx);
    ret = scil_destroy_context(ctx);
    DEBUG("SCIL compressed: %ld => %ld\n", f->bytes, out_size);
    if(out_size < f->bytes){
      // no need to use bigger data
      f->actual_bytes = out_size;
      // update data to write
      bytes_to_write = out_size;
      writeBuffer = scil_buf;
    }
#endif
*/
