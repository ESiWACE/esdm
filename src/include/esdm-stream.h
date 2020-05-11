#include <esdm-datatypes.h>

typedef struct esdm_wstream_metadata_t esdm_wstream_metadata_t;

#define defineStreamType(streamType, elementType) typedef struct streamType { \
  esdm_wstream_metadata_t* metadata; /*contains opaque implementation details of the stream API*/ \
  elementType *buffer, *iter, *iterEnd, *bufferEnd; \
} streamType
defineStreamType(esdm_wstream_uint8_t, uint8_t);
defineStreamType(esdm_wstream_uint16_t, uint16_t);
defineStreamType(esdm_wstream_uint32_t, uint32_t);
defineStreamType(esdm_wstream_uint64_t, uint64_t);
defineStreamType(esdm_wstream_int8_t, int8_t);
defineStreamType(esdm_wstream_int16_t, int16_t);
defineStreamType(esdm_wstream_int32_t, int32_t);
defineStreamType(esdm_wstream_int64_t, int64_t);
defineStreamType(esdm_wstream_float_t, float_t);
defineStreamType(esdm_wstream_double_t, double_t);
#undef defineStreamType

//We could use `gcc`'s typeof extension, but that's not portable.
//So we define our own variant just for our own types.
#define esdm_typeof(type) _Generic(type, \
  esdm_wstream_uint8_t : esdm_wstream_uint8_t , \
  esdm_wstream_uint16_t: esdm_wstream_uint16_t, \
  esdm_wstream_uint32_t: esdm_wstream_uint32_t, \
  esdm_wstream_uint64_t: esdm_wstream_uint64_t, \
  esdm_wstream_int8_t  : esdm_wstream_int8_t  , \
  esdm_wstream_int16_t : esdm_wstream_int16_t , \
  esdm_wstream_int32_t : esdm_wstream_int32_t , \
  esdm_wstream_int64_t : esdm_wstream_int64_t , \
  esdm_wstream_float_t : esdm_wstream_float_t , \
  esdm_wstream_double_t: esdm_wstream_double_t, \
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

/**
 * Setup a stream for writing data to a dataset.
 *
 * @param [inout] stream pointer to one of the stream types `esdm_wstream_uint8_t` through `esdm_wstream_double_t`
 * @param [in] dataset pointer to the dataset to which the data is to be written
 * @param [in] dimCount must equal the dim count of the dataset, also the assumed size of the `offset` and `size` arrays
 * @param [in] offset array of `dimCount` elements that provides the logical coordinates of the first value that will be streamed
 * @param [in] size array of `dimCount` elements that provides the extends of the hypercube that is to be streamed
 *
 * Typical usage:
 *
 *     esdm_wstream_double_t stream;
 *     esdm_wstream_start(&stream, dataset, 2, (int64_t[2]){50, 72}, (int64_t[2]){250, 36});
 *     for(int y = 50; y < 300; y++) {
 *         for(int x = 72; x < 108; x++) {
 *             esdm_wstream_pack(stream, computeValueForLocation(x, y));
 *         }
 *     }
 *     esdm_wstream_commit(stream);
 */
#define esdm_wstream_start(stream, dataset, dimCount, offset, size) do { \
  esdm_typeof(stream)* const esdm_internal_stream_ptr = &(stream); /*avoid multiple evaluation*/ \
  esdm_wstream_metadata_t* esdm_internal_stream_metadata = esdm_wstream_metadata_create(dataset, dimCount, offset, size, smd_c_to_smd_type(*esdm_internal_stream_ptr->buffer)); \
  int64_t esdm_internal_element_count = esdm_wstream_metadata_max_chunk_size(esdm_internal_stream_metadata); \
  esdm_typeof(*stream->buffer) esdm_internal_buffer = malloc(esdm_internal_element_count*sizeof*esdm_internal_stream_ptr->buffer); \
  *esdm_internal_stream_ptr = (esdm_typeof(*stream)){ \
    .buffer = esdm_internal_buffer, \
    .bufferEnd = esdm_internal_buffer + esdm_internal_element_count, \
    .iter = esdm_internal_buffer, \
    .iterEnd = esdm_internal_buffer + esdm_wstream_metadata_next_chunk_size(esdm_internal_stream_metadata), \
    .metadata = esdm_internal_stream_metadata \
  } \
} while(0)

/**
 * Push a single value into stream.
 *
 * @param[in] stream the stream to write to
 * @param[in] value the value to add to the stream
 *
 * It is an error to push more values into the stream than what was requested in the corresponding `esdm_wstream_start()` call.
 * See `esdm_wstream_start()` for a usage example.
 */
#define esdm_wstream_pack(stream, value) do { \
  esdm_typeof(stream)* const esdm_internal_stream_ptr = &(stream); /*avoid multiple evaluation*/ \
  if(esdm_internal_stream_ptr->iter >= esdm_internal_stream_ptr->iterEnd) { \
    fprintf(stderr, "attempt to push more data into a stream than defined at stream creation\n"); \
    abort(); \
  } \
  *esdm_internal_stream_ptr->iter++ = (value); \
  if(esdm_internal_stream_ptr->iter == esdm_internal_stream_ptr->iterEnd) { \
    esdm_wstream_flush(esdm_internal_stream_ptr->metadata, esdm_internal_stream_ptr->buffer, esdm_internal_stream_ptr->iter); \
    esdm_internal_stream_ptr->iter = esdm_internal_stream_ptr->buffer; \
    esdm_internal_stream_ptr->iterEnd = esdm_internal_stream_ptr->buffer + esdm_wstream_metadata_next_chunk_size(esdm_internal_stream_metadata); \
  } \
}

/**
 * Signal the end of the streaming and perform any required cleanup.
 *
 * @param[in] stream the stream to commit, close, and destroy
 *
 * It is an error to call this macro before the stream has been fully packed with data.
 * See `esdm_wstream_start()` for a usage example.
 */
#define esdm_wstream_commit(stream) do { \
  esdm_typeof(stream)* const esdm_internal_stream_ptr = &(stream); /*avoid multiple evaluation*/ \
  if(esdm_internal_stream_ptr->iterEnd != esdm_internal_stream_ptr->buffer) { \
    fprintf(stderr, "preliminary commit of a stream: too few calls to esdm_wstream_pack()\n"); \
    abort(); \
  } \
  /*since `esdm_wstream_pack()` flushes the stream *after* adding the last value, we only need to perform local cleanup*/ \
  esdm_wstream_metadata_destroy(esdm_internal_stream_ptr->metadata); \
  free(esdm_internal_stream_ptr->buffer); \
  *esdm_internal_stream_ptr = (esdm_typeof(*stream)){0}; \
} while(0)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal API ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Create the opaque metadata object for a write stream.
 *
 * This is an internal function that should not be used directly by user code, use the `esdm_wstream_start()` macro instead.
 */
esdm_wstream_metadata_t* esdm_wstream_metadata_create(esdm_dataset_t* dataset, int64_t dimCount, int64_t* offset, int64_t* size, esdm_type_t type);

/**
 * Query the stream metadata object for the largest chunk size that needs to be fed into the stream.
 *
 * This is an internal function that should not be used directly by user code, use the `esdm_wstream_start()` macro instead.
 */
int64_t esdm_wstream_metadata_max_chunk_size(esdm_wstream_metadata_t* metadata);

/**
 * Query the stream metadata object for the size of the next chunk that needs to be fed into the stream.
 *
 * This is an internal function that should not be used directly by user code, use the `esdm_wstream_start()` and `esdm_wstream_pack()` macros instead.
 */
int64_t esdm_wstream_metadata_next_chunk_size(esdm_wstream_metadata_t* metadata);

/**
 * Forward a chunk of data for further processing from a stream.
 *
 * This is an internal function that should not be used directly by user code, use the `esdm_wstream_pack()` macro instead.
 *
 * `(streamType*)bufferEnd - (streamType*)buffer` should equal the last value returned by `esdm_wstream_metadata_next_chunk_size()`.
 */
void esdm_wstream_flush(esdm_wstream_metadata_t* metadata, void* buffer, void* bufferEnd);

/**
 * Get rid of a stream's metadata.
 *
 * This is an internal function that should not be used directly by user code, use the `esdm_wstream_commit()` macro instead.
 */
void esdm_wstream_metadata_destroy(esdm_wstream_metadata_t* metadata);
