/**
* This test uses the generic patterns to verify the high-level interface works as intended
*/

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
    int64_t dim[] = {50, 100};
    dataspace = esdm_dataspace_create(2, dim, ESDM_TYPE_UINT64_T);
  }

	esdm_container_t * container = esdm_container_create("testContainer");
	esdm_dataset_t   * dataset   = esdm_dataset_create(container, "testDataset", dataspace);

  esdm_dataspace_t* s1, * s2, * s3, *s4;
  esdm_fragment_t * f1, * f2, *f3, *f4;

  {
    int64_t offset[] = {0, 0};
    int64_t dim[] = {25, 50};
	  s1 = esdm_dataspace_subspace(dataspace, 2, dim, offset);
    f1 = esdm_fragment_create(dataset, s1, buff);
  }
  {
    int64_t offset[] = {25, 0};
    int64_t dim[] = {25, 50};
	  s2 = esdm_dataspace_subspace(dataspace, 2, dim, offset);
    f2 = esdm_fragment_create(dataset, s2, buff);
  }
  {
    int64_t offset[] = {25, 50};
    int64_t dim[] = {25, 50};
	  s3 = esdm_dataspace_subspace(dataspace, 2, dim, offset);
    f3 = esdm_fragment_create(dataset, s3, buff);
  }
  {
    int64_t offset[] = {0, 50};
    int64_t dim[] = {25, 50};
	  s4 = esdm_dataspace_subspace(dataspace, 2, dim, offset);
    f4 = esdm_fragment_create(dataset, s4, buff);
  }

  b->callbacks.fragment_update(b, f1);
  b->callbacks.fragment_update(b, f2);
  b->callbacks.fragment_update(b, f3);
  b->callbacks.fragment_update(b, f4);
  //fragment_retrieve


  b->callbacks.finalize(b);

  esdm_dataspace_destroy(dataspace);
  esdm_dataset_destroy(dataset);
  esdm_container_destroy(container);

  return 0;
}
