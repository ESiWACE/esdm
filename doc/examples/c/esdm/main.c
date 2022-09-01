#include <esdm.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  uint64_t *buf_w = malloc(10 * 20 * sizeof(uint64_t));

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      buf_w[y * 10 + x] = (y)*10 + x + 1;
    }
  }

  // Interaction with ESDM
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  esdm_status status = esdm_init();
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);

  // define dataspace
  int64_t bounds[] = {10, 20};
  esdm_dataspace_t *dataspace;

  status = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);

  status = esdm_container_create("/po/test", 1, &container);
  status = esdm_container_close(container);
  status = esdm_container_create("po/test/", 1, &container);
  status = esdm_container_close(container);
  status = esdm_container_create("mycontainer", 1, &container);

  status = esdm_dataset_create(container, "/mydataset", dataspace, &dataset);
  status = esdm_dataset_create(container, "öüö&asas", dataspace, &dataset);
  status = esdm_dataset_create(container, "test/", dataspace, &dataset);
  status = esdm_dataset_create(container, "test//test", dataspace, &dataset);
  status = esdm_dataset_create(container, "/test/test", dataspace, &dataset);
  status = esdm_dataset_create(container, "test/test", dataspace, &dataset);
  status = esdm_dataset_close(dataset);
  status = esdm_dataset_create(container, "mydataset", dataspace, &dataset);
  status = esdm_container_commit(container);
  status = esdm_dataset_commit(dataset);

  // define subspace
  int64_t size[] = {10, 20};
  int64_t offset[] = {0, 0};
  esdm_dataspace_t *subspace;

  status = esdm_dataspace_subspace(dataspace, 2, size, offset, &subspace);

  // Write the data to the dataset
  printf("Write 0\n");
  printf("Write 1\n");
  printf("Write 2\n");
  printf("Write 3\n");
  status = esdm_write(dataset, buf_w, subspace);
  status = esdm_container_commit(container);
  status = esdm_dataset_commit(dataset);
  status = esdm_dataset_close(dataset);
  status = esdm_container_close(container);
  status = esdm_finalize();

  // clean up
  free(buf_w);
  status = esdm_dataspace_destroy(dataspace);
  status = esdm_dataspace_destroy(subspace);

  printf("\nOK\n");
  return 0;
}
