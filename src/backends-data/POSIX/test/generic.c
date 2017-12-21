/**
* This test uses the generic patterns to verify the high-level interface works as intended
*/

#include <data-backends/test/generic-backend.h>
#include <data-backends/POSIX/posix.h>

int main(){
  // TODO: setup backend environment
  ESDM_backend_t backend = ....;

  generic_test(backend, ...);

  // TODO: teardown backend environment
  return 0;
}
