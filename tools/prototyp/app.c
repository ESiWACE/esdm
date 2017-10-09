#include <mpi.h>

/*
 * Test: scenario
 * 1 container, 1 Variable, N*Fragments
 * write fragments locally to /dev/shm or /tmp
 * read fragments locally from device
 * alternative: store it to a single shared file
 *
 * Tests: small run of the benchmark => small fragments => local memory
 *        large run of the benchmark => large fragments => /tmp
 *    extreme large                  => lustre
 *
 * configuration:
 ** /dev/shm [local]
 ** /tmp [local]
 ** /pfs/ [shared]
 *
 * NetCDF benchmark use
 * change API calls to some esd-mpi calls
 */

int main(){
  return 0;
}
