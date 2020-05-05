#ifndef ESDM_STREAM_H
#define ESDM_STREAM_H

#include <esdm-datatypes.h>

struct estream_write_t {
  esdm_fragment_t * fragment;
  void * backend_state; // backend may store anything here for streaming
};

#endif
