/* This file is part of ESDM. See the enclosed LICENSE */

#include <test/util/test_util.h>

#include <esdm-internal.h>
#include <esdm.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_SCIL
#include <scil.h>
#endif

int main(int argc, char const *argv[]) {
  // prepare data
  uint64_t *buf_w = ea_checked_malloc(10 * 20 * sizeof(uint64_t));
  uint64_t *buf_r = ea_checked_malloc(10 * 20 * sizeof(uint64_t));

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 20; y++) {
      buf_w[y * 10 + x] = (y)*10 + x + 1;
    }
  }

  // Interaction with ESDM
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;

  esdm_status status = esdm_init();
  eassert(status == ESDM_SUCCESS);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
  eassert(status == ESDM_SUCCESS);
  status = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
  eassert(status == ESDM_SUCCESS);

  // define dataspace
  int64_t bounds[] = {10, 20};
  esdm_dataspace_t *dataspace;
  status = esdm_dataspace_create(2, bounds, SMD_DTYPE_UINT64, &dataspace);
  eassert(status == ESDM_SUCCESS);

  status = esdm_container_create("scil", 1, &container);
  eassert(status == ESDM_SUCCESS);

  status = esdm_dataset_create(container, "test1", dataspace, &dataset);
  eassert(status == ESDM_SUCCESS);

  #define ABSTOL 1

#ifdef HAVE_SCIL
  scil_user_hints_t hints;
  scil_user_hints_initialize(& hints);
  hints.absolute_tolerance = ABSTOL;
  status = esdm_dataset_set_compression_hint(dataset, & hints);
  eassert(status == ESDM_SUCCESS);
#else
  scil_user_hints_t * hints = NULL;
  status = esdm_dataset_set_compression_hint(dataset, hints);
  eassert(status == ESDM_ERROR);
#endif

  status = esdm_container_commit(container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  // define subspace
  int64_t size[] = {10, 20};
  int64_t offset[] = {0, 0};
  esdm_dataspace_t *subspace;

  status = esdm_dataspace_subspace(dataspace, 2, size, offset, &subspace);
  eassert(status == ESDM_SUCCESS);
  status = esdm_write(dataset, buf_w, subspace);
  eassert(status == ESDM_SUCCESS);

  status = esdm_container_commit(container);
  eassert(status == ESDM_SUCCESS);
  status = esdm_dataset_commit(dataset);
  eassert(status == ESDM_SUCCESS);

  status = esdm_read(dataset, buf_r, subspace);
  eassert(status == ESDM_SUCCESS);
  for (int x = 0; x < 200; x++) {
    eassert(buf_r[x] >= (buf_w[x] - ABSTOL) && buf_r[x] <= (buf_w[x] + ABSTOL));
  }

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);
  // clean up
  free(buf_w);

  printf("\nOK\n");

  return 0;
}
