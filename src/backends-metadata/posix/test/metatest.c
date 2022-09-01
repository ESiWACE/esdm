/**
* This test uses the generic patterns to verify the high-level interface works as intended
*/


#include <backends-metadata/posix/md-posix.h>

int main() {
  esdm_status ret;
  char const * cfg = "{\"esdm\": {\"backends\": ["
  		"{"
			"\"type\": \"DUMMY\","
			"\"id\": \"p1\","
			"\"target\": \"x\""
			"}"
    "],"
		"\"metadata\": {"
			"\"type\": \"metadummy\","
			"\"id\": \"md\","
			"\"target\": \"./_metadummy\"}}"
    "}";
  esdm_load_config_str(cfg);
  esdm_init();

  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(ret == ESDM_SUCCESS);

  esdm_simple_dspace_t dataspace = esdm_dataspace_2d(50, 100, SMD_DTYPE_UINT64);
  eassert(dataspace.ptr);
  esdm_container_t *container;

  ret = esdm_container_create("testContainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);
  esdm_dataset_t *dataset;

  ret = esdm_dataset_create(container, "testDataset", dataspace.ptr, &dataset);
  eassert(ret == ESDM_SUCCESS);

  esdm_fragment_t *f1, *f2, *f3, *f4;

  {
    esdm_simple_dspace_t space = esdm_dataspace_2do(0, 25, 0, 50, SMD_DTYPE_UINT64);
    ret = esdmI_fragment_create(dataset, space.ptr, ea_checked_malloc(esdm_dataspace_total_bytes(space.ptr)), &f1);
    eassert(ret == ESDM_SUCCESS);
  }
  {
    esdm_simple_dspace_t space = esdm_dataspace_2do(25, 25, 0, 50, SMD_DTYPE_UINT64);
    ret = esdmI_fragment_create(dataset, space.ptr, ea_checked_malloc(esdm_dataspace_total_bytes(space.ptr)), &f2);
    eassert(ret == ESDM_SUCCESS);
  }
  {
    esdm_simple_dspace_t space = esdm_dataspace_2do(25, 25, 50, 50, SMD_DTYPE_UINT64);
    ret = esdmI_fragment_create(dataset, space.ptr, ea_checked_malloc(esdm_dataspace_total_bytes(space.ptr)), &f3);
    eassert(ret == ESDM_SUCCESS);
  }
  {
    esdm_simple_dspace_t space = esdm_dataspace_2do(0, 25, 50, 50, SMD_DTYPE_UINT64);
    ret = esdmI_fragment_create(dataset, space.ptr, ea_checked_malloc(esdm_dataspace_total_bytes(space.ptr)), &f4);
    eassert(ret == ESDM_SUCCESS);
  }

  //ret = esdmI_backend_fragment_update(b, f1);
  //eassert(ret == ESDM_SUCCESS);
  //ret = esdmI_backend_fragment_update(b, f2);
  //eassert(ret == ESDM_SUCCESS);
  //ret = esdmI_backend_fragment_update(b, f3);
  //eassert(ret == ESDM_SUCCESS);
  //ret = esdmI_backend_fragment_update(b, f4);
  //eassert(ret == ESDM_SUCCESS);

  //{
  //  int64_t size[] = {30, 30};
  //  int64_t offset[] = {10, 10};
  //  esdm_dataspace_subspace(dataspace, 2, size, offset, &res);
  //}

  esdmI_container_destroy(container);

  esdm_finalize();

  return 0;
}
