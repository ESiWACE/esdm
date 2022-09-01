#include <test/util/test_util.h>

#include <esdm-internal.h>
#include <esdm.h>
#include <string.h>

int main() {
  int64_t dim[2] = {10, 20}; // dim order highest first: (y, x)
  int64_t dim2[2] = {5, 20};

  float *p = ea_checked_malloc(sizeof(float) * dim[0] * dim[1] * 2);
  float **data = ea_checked_malloc(sizeof(float *) * dim[1]);
  float **data_out = ea_checked_malloc(sizeof(float *) * dim[1]);

  // prepare test data
  float *c = p;
  for (int y = 0; y < dim[0]; y++) {
    data[y] = c;
    for (int x = 0; x < dim[1]; x++) {
      data[y][x] = x * y + x + 0.5;
    }
    c += dim[1];
  }
  for (int y = 0; y < dim[0]; y++) {
    data_out[y] = c;
    for (int x = 0; x < dim[1]; x++) {
      data_out[y][x] = 0;
    }
    c += dim[1];
  }

  esdm_status ret;
  esdm_dataspace_t *space;

  esdm_dataspace_create(2, dim, SMD_DTYPE_FLOAT, &space);
  eassert(space != NULL);

  // walk through all offsets
  for (int o = 0; o < dim[0] / dim2[0]; o++) {
    int offsetY = o * dim2[0];
    int64_t offset[2] = {offsetY, 0};

    esdm_dataspace_t *subspace;

    esdm_dataspace_subspace(space, 2, dim2, offset, &subspace);
    eassert(subspace != NULL);

    uint64_t size = esdm_dataspace_total_bytes(subspace);
    printf("Offset: %d,%d -- size: %lu\n", (int)offset[0], (int)offset[1], size);

    // now copy the data from the position
    uint64_t off_buff = offset[0];
    for (int i = 0; i < space->dims; i++) {
      off_buff *= space->size[i];
    }
    off_buff *= esdm_sizeof(subspace->type);

    printf("Buffer offset: %lu %zu %zu\n", off_buff, (size_t)((char *)data_out[0]) + off_buff, (size_t)((char *)data[0]) + off_buff);
    memcpy(((char *)data_out[0]) + off_buff, ((char *)data[0]) + off_buff, size);

    // compare results:
    for (int y = offset[0]; y < dim2[0] + offsetY; y++) {
      for (int x = offset[1]; x < dim2[1]; x++) {
        double delta = (double)(data[y][x] - data_out[y][x]);
        printf("%4.0f", delta);
      }
      printf("\n");
    }
    ret = esdm_dataspace_destroy(subspace);
    eassert(ret == ESDM_SUCCESS);
  }

  eassert_crash(esdm_dataspace_destroy(NULL));
  ret = esdm_dataspace_destroy(space);
  eassert(ret == ESDM_SUCCESS);

  free(p);
  free(data);
  free(data_out);

  printf("\nOK\n");

  return 0;
}
