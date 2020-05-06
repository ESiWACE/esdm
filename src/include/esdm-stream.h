#ifndef ESDM_STREAM_H
#define ESDM_STREAM_H

#include <esdm-datatypes.h>

struct estream_write_t {
  esdm_fragment_t * fragment;
  void * backend_state; // backend may store anything here for streaming
};

/*
 * Pack the whole data of the fragment at once into outbuffer, potentially apply compression
 *
 * @param in_out_buff If *in_out_buff is != NULL, it is a pointer to a memory region where the output is stored. The output memory region must be large enough. If it is NULL, the function will return a pointer to the memory region that must be used. If it is != f->buffer, then it must be freed.
 * @param out_size The actual capacity used of the output buffer.
 */
int estream_mem_pack_fragment(esdm_fragment_t *f, void ** in_out_buff, size_t * out_size);


bool estream_mem_unpack_fragment_param(esdm_fragment_t *f, void ** out_buf, size_t * out_size);
/*
 * Reverse function, takes the read buffer and stuffs the data into the output buffer
 */
int estream_mem_unpack_fragment(esdm_fragment_t *f, void * rbuff, size_t size);

#endif
