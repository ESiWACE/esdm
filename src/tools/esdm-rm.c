#include <esdm.h>
#include <stdlib.h>

#include "tool-util.h"

/**
 * @file
 * @brief rm ESDM containers.
 */
int main(int argc, char **argv) {
  if(argc != 2){
    printf("Syntax: %s [CONTAINER]\n", argv[0]);
    return 1;
  }

  int ret;
  ret = esdm_init();
  if(ret != ESDM_SUCCESS) ESDM_ERROR("Cannot initialize");


  esdm_container_t * container;
  ret = esdm_container_open(argv[1], ESDM_MODE_FLAG_WRITE, &container);
  if(ret != ESDM_SUCCESS) ESDM_ERROR_FMT("Cannot open the container %s", argv[1]);

  ret = esdm_container_delete(container);
  if(ret != ESDM_SUCCESS) ESDM_ERROR_FMT("Cannot delete the container %s", argv[1]);

  ret = esdm_finalize();
  if(ret != ESDM_SUCCESS) ESDM_ERROR("Error in finalize");

  return 0;
}
