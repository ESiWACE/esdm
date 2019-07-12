/**
* This test uses the generic patterns to verify the high-level interface works as intended
*/


#include <backends-metadata/metadummy/metadummy.h>

extern esdm_instance_t esdm;

int main() {
  esdm_status ret;
  char const * cfg = "{\"esdm\": {\"backends\": [],"
		"\"metadata\": {"
			"\"type\": \"metadummy\","
			"\"id\": \"md\","
			"\"target\": \"./_metadummy\"}}}";
  esdm_load_config_str(cfg);
  esdm_init();
  esdm_md_backend_t *b = esdm.modules->metadata_backend;

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  char *buff = "test";
  esdm_dataspace_t *dataspace;
  {
    int64_t size[] = {50, 100};
    esdm_dataspace_create(2, size, SMD_DTYPE_UINT64, &dataspace);
  }
  esdm_container_t *container;

  ret = esdm_container_create("testContainer", &container);
  eassert(ret == ESDM_SUCCESS);
  esdm_dataset_t *dataset;

  ret = esdm_dataset_create(container, "testDataset", dataspace, &dataset);
  eassert(ret == ESDM_SUCCESS);

  esdm_dataspace_t *s1, *s2, *s3, *s4;
  esdm_fragment_t *f1, *f2, *f3, *f4;

  {
    int64_t offset[] = {0, 0};
    int64_t size[] = {25, 50};
    ret = esdm_dataspace_subspace(dataspace, 2, size, offset, &s1);
    eassert(ret == ESDM_SUCCESS);
    ret = esdmI_fragment_create(dataset, s1, buff, &f1);
    eassert(ret == ESDM_SUCCESS);
  }
  {
    int64_t offset[] = {25, 0};
    int64_t size[] = {25, 50};
    ret = esdm_dataspace_subspace(dataspace, 2, size, offset, &s2);
    eassert(ret == ESDM_SUCCESS);
    ret = esdmI_fragment_create(dataset, s2, buff, &f2);
    eassert(ret == ESDM_SUCCESS);
  }
  {
    int64_t offset[] = {25, 50};
    int64_t size[] = {25, 50};
    ret = esdm_dataspace_subspace(dataspace, 2, size, offset, &s3);
    eassert(ret == ESDM_SUCCESS);
    ret = esdmI_fragment_create(dataset, s3, buff, &f3);
    eassert(ret == ESDM_SUCCESS);
  }
  {
    int64_t offset[] = {0, 50};
    int64_t size[] = {25, 50};
    ret = esdm_dataspace_subspace(dataspace, 2, size, offset, &s4);
    eassert(ret == ESDM_SUCCESS);
    ret = esdmI_fragment_create(dataset, s4, buff, &f4);
    eassert(ret == ESDM_SUCCESS);
  }

  //ret = b->callbacks.fragment_update(b, f1);
  //eassert(ret == ESDM_SUCCESS);
  //ret = b->callbacks.fragment_update(b, f2);
  //eassert(ret == ESDM_SUCCESS);
  //ret = b->callbacks.fragment_update(b, f3);
  //eassert(ret == ESDM_SUCCESS);
  //ret = b->callbacks.fragment_update(b, f4);
  //eassert(ret == ESDM_SUCCESS);

  //{
  //  int64_t size[] = {30, 30};
  //  int64_t offset[] = {10, 10};
  //  esdm_dataspace_subspace(dataspace, 2, size, offset, &res);
  //}

  b->callbacks.finalize(b);

  esdm_dataspace_destroy(dataspace);
  esdmI_dataset_destroy(dataset);

  esdmI_container_destroy(container);

  esdm_finalize();

  return 0;
}
