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

#endif
