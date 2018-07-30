#include <esdm.h>
#include <esdm-internal.h>

int main(){
  int64_t dim[2]    = {10, 20}; // dim order highest first: (y, x)
  int64_t dim2[2]   = {5, 20};

  float * p         = malloc(sizeof(float) * dim[0] * dim[1] * 2);
  float ** data     = malloc(sizeof(float*) * dim[1]);
  float ** data_out = malloc(sizeof(float*) * dim[1]);

  // prepare test data
  float * c = p;
  for(int y = 0; y < dim[0]; y++){
    data[y] = c;
    for(int x = 0; x < dim[1]; x++){
      data[y][x] = x*y + x + 0.5;
    }
    c += dim[1];
  }
  for(int y = 0; y < dim[0]; y++){
    data_out[y] = c;
    for(int x = 0; x < dim[1]; x++){
      data_out[y][x] = 0;
    }
    c += dim[1];
  }


  esdm_status_t ret;
  esdm_dataspace_t* space = esdm_dataspace_create(2, dim, ESDM_TYPE_FLOAT);
  assert(space != NULL);

  // walk through all offsets
  for(int o = 0; o < dim[0] / dim2[0]; o++){
    int offsetY = o * dim2[0];
    int64_t offset[2] = {offsetY, 0};

  	esdm_dataspace_t* subspace = esdm_dataspace_subspace(space, 2, dim2, offset);
    assert(subspace != NULL);

    uint64_t size = esdm_dataspace_size(subspace);
    printf("Offset: %d,%d -- size: %lu\n", (int) offset[0], (int) offset[1], size);

    // now copy the data from the position
    uint64_t off_buff = esdm_buffer_offset_first_dimension(space, offset[0]);
    printf("Buffer offset: %lu %zu %zu\n", off_buff, ((char*) data_out[0]) + off_buff, ((char*) data[0]) + off_buff);
    memcpy(((char*) data_out[0]) + off_buff, ((char*) data[0]) + off_buff, size);

    // compare results:
    for(int y = offset[0]; y < dim2[0] + offsetY; y++){
      for(int x = offset[1]; x < dim2[1]; x++){
        double delta = (double) (data[y][x] - data_out[y][x]);
        printf("%4.0f", delta);
      }
      printf("\n");
    }
    ret = esdm_dataspace_destroy(subspace);
    assert(ret == ESDM_SUCCESS);
  }

  ret = esdm_dataspace_destroy(space);
  assert(ret == ESDM_SUCCESS);

  free(p);
  free(data);
  free(data_out);

  return 0;
}
