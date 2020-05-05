typedef struct esdm_stream_metadata_t esdm_stream_metadata_t;

typedef struct esdm_stream_uint8_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  uint8_t *start, *current, *end;
} esdm_stream_uint8_t;

typedef struct esdm_stream_uint16_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  uint16_t *start, *current, *end;
} esdm_stream_uint16_t;

typedef struct esdm_stream_uint32_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  uint32_t *start, *current, *end;
} esdm_stream_uint32_t;

typedef struct esdm_stream_uint64_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  uint64_t *start, *current, *end;
} esdm_stream_uint64_t;

typedef struct esdm_stream_int8_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  int8_t *start, *current, *end;
} esdm_stream_int8_t;

typedef struct esdm_stream_int16_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  int16_t *start, *current, *end;
} esdm_stream_int16_t;

typedef struct esdm_stream_int32_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  int32_t *start, *current, *end;
} esdm_stream_int32_t;

typedef struct esdm_stream_int64_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  int64_t *start, *current, *end;
} esdm_stream_int64_t;

typedef struct esdm_stream_float_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  float *start, *current, *end;
} esdm_stream_float_t;

typedef struct esdm_stream_double_t {
  esdm_stream_metadata_t* metadata; //contains opaque implementation details of the stream API
  double *start, *current, *end;
} esdm_stream_double_t;

//We could use `gcc`'s typeof extension, but that's not portable.
//So we define our own variant just for our own types.
#define esdm_typeof(type) _Generic(type, \
  esdm_stream_uint8_t : esdm_stream_uint8_t , \
  esdm_stream_uint16_t: esdm_stream_uint16_t, \
  esdm_stream_uint32_t: esdm_stream_uint32_t, \
  esdm_stream_uint64_t: esdm_stream_uint64_t, \
  esdm_stream_int8_t  : esdm_stream_int8_t  , \
  esdm_stream_int16_t : esdm_stream_int16_t , \
  esdm_stream_int32_t : esdm_stream_int32_t , \
  esdm_stream_int64_t : esdm_stream_int64_t , \
  esdm_stream_float_t : esdm_stream_float_t , \
  esdm_stream_double_t: esdm_stream_double_t, \
  uint8_t : uint8_t , \
  uint16_t: uint16_t, \
  uint32_t: uint32_t, \
  uint64_t: uint64_t, \
  int8_t  : int8_t  , \
  int16_t : int16_t , \
  int32_t : int32_t , \
  int64_t : int64_t , \
  float   : float   , \
  double  : double)

esdm_status esdm_write_req_start_uint8 (esdm_stream_uint8_t*  stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_uint16(esdm_stream_uint16_t* stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_uint32(esdm_stream_uint32_t* stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_uint64(esdm_stream_uint64_t* stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_int8  (esdm_stream_int8_t*   stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_int16 (esdm_stream_int16_t*  stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_int32 (esdm_stream_int32_t*  stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_int64 (esdm_stream_int64_t*  stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_float (esdm_stream_float_t*  stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);
esdm_status esdm_write_req_start_double(esdm_stream_double_t* stream, esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size);

#define esdm_write_req_start(stream, dataset, dimCount, offset, size) _Generic(stream, \
  esdm_stream_uint8_t* : esdm_write_req_start_uint8 , \
  esdm_stream_uint16_t*: esdm_write_req_start_uint16, \
  esdm_stream_uint32_t*: esdm_write_req_start_uint32, \
  esdm_stream_uint64_t*: esdm_write_req_start_uint64, \
  esdm_stream_int8_t*  : esdm_write_req_start_int8  , \
  esdm_stream_int16_t* : esdm_write_req_start_int16 , \
  esdm_stream_int32_t* : esdm_write_req_start_int32 , \
  esdm_stream_int64_t* : esdm_write_req_start_int64 , \
  esdm_stream_float_t* : esdm_write_req_start_float , \
  esdm_stream_double_t*: esdm_write_req_start_double)(stream, dataset, dimCount, offset, size)

#define esdm_write_req_add(stream, value) do { \
  esdm_typeof(stream)* const esdm_internal_stream_ptr = &(stream); /*avoid multiple evaluation*/ \
  if(esdm_internal_stream_ptr->current == esdm_internal_stream_ptr->end) esdm_write_req_flush(esdm_internal_stream_ptr); \
  *esdm_internal_stream_ptr->current++ = (value); \
}
