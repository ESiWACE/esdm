/**
* This test uses the generic patterns to verify the high-level interface works as intended
*/

#include <assert.h>

#include <backends-metadata/metadummy/metadummy.h>

int main(){
  esdm_config_backend_t config = {
    	.type = "metadummy",
    	.id = "test1",
    	.target = "_metadummy-test",
  };

  esdm_backend_t* b = metadummy_backend_init(& config);

  char * buff = "test";
  esdm_dataspace_t * dataspace;
  {
    int64_t size[]   = {50, 100};
    dataspace = esdm_dataspace_create(2, size, ESDM_TYPE_UINT64_T);
  }

  esdm_status ret;
	esdm_container_t * container = esdm_container_create("testContainer");
	esdm_dataset_t   * dataset   = esdm_dataset_create(container, "testDataset", dataspace);

  esdm_dataspace_t* s1, * s2, * s3, *s4;
  esdm_fragment_t * f1, * f2, *f3, *f4;

  {
    int64_t offset[] = {0, 0};
    int64_t size[]   = {25, 50};
	  s1 = esdm_dataspace_subspace(dataspace, 2, size, offset);
    f1 = esdm_fragment_create(dataset, s1, buff);
    f1->metadata->size = sprintf(f1->metadata->json, "{}");
  }
  {
    int64_t offset[] = {25, 0};
    int64_t size[]   = {25, 50};
	  s2 = esdm_dataspace_subspace(dataspace, 2, size, offset);
    f2 = esdm_fragment_create(dataset, s2, buff);
  }
  {
    int64_t offset[] = {25, 50};
    int64_t size[]   = {25, 50};
	  s3 = esdm_dataspace_subspace(dataspace, 2, size, offset);
    f3 = esdm_fragment_create(dataset, s3, buff);
  }
  {
    int64_t offset[] = {0, 50};
    int64_t size[]   = {25, 50};
	  s4 = esdm_dataspace_subspace(dataspace, 2, size, offset);
    f4 = esdm_fragment_create(dataset, s4, buff);
  }

  ret = b->callbacks.fragment_update(b, f1);
  assert(ret == ESDM_SUCCESS);
  ret = b->callbacks.fragment_update(b, f2);
  assert(ret == ESDM_SUCCESS);
  ret = b->callbacks.fragment_update(b, f3);
  assert(ret == ESDM_SUCCESS);
  ret = b->callbacks.fragment_update(b, f4);
  assert(ret == ESDM_SUCCESS);

  //fragment_retrieve
  esdm_dataspace_t * res;
  int frag_count;
  esdm_fragment_t ** read_frag = NULL;

  {
    int64_t size[]   = {30, 30};
    int64_t offset[] = {10, 10};
	  res = esdm_dataspace_subspace(dataspace, 2, size, offset);
  }

  ret = b->callbacks.lookup(b, dataset, res, & frag_count, & read_frag);
  assert(ret == ESDM_SUCCESS);
  printf("Found fragments: %d\n", frag_count);
  assert(frag_count == 2);
  for(int i=0; i < frag_count; i++){
    esdm_fragment_print(read_frag[i]);
    printf("\n");
  }

  b->callbacks.finalize(b);

  esdm_dataspace_destroy(dataspace);
  esdm_dataset_destroy(dataset);
  esdm_container_destroy(container);

  return 0;
}
