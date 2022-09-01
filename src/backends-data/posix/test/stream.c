/**
* This test uses the generic patterns to verify the high-level interface works as intended
*/


#include <backends-data/posix/posix.h>
#include <esdm-stream.h>

int main() {
  esdm_status ret;

  esdm_config_backend_t * cfg = ea_checked_malloc(sizeof(esdm_config_backend_t));
  esdm_config_backend_t orig = {
    .type = "POSIX",
    .target = "./posix"};
  memcpy(cfg, & orig, sizeof(orig));

  esdm_backend_t * b = posix_backend_init(cfg);
  assert(b);

  esdmI_backend_mkfs(b, ESDM_FORMAT_CREATE|ESDM_FORMAT_DELETE|ESDM_FORMAT_IGNORE_ERRORS);

  esdm_simple_dspace_t dspace = esdm_dataspace_2d(1024, 10, SMD_DTYPE_UINT8);

  esdm_dataset_t dset = {.name = "test", .id = "testID", .dataspace = dspace.ptr};
  esdm_fragment_t frag = { .dataset = & dset, .id = NULL, .dataspace = dspace.ptr,
    .bytes = 10*1024, ESDM_DATA_DIRTY, .backend = b };

  void * buff = ea_checked_malloc(1024);
  for(int i=0; i < 1024; i++){
    ((uint8_t*) buff)[i] = (uint8_t) (i % 256);
  }

  estream_write_t state = { .fragment = & frag };
  for(int i=0; i < 10; i++){
    ret = esdmI_backend_fragment_write_stream_blocksize(b, & state, buff, 1024*i, 1024);
    assert(ret == ESDM_SUCCESS);
  }

  ret = posix_finalize(b);
  assert(ret == ESDM_SUCCESS);

  printf("OK\n");
  return 0;
}
