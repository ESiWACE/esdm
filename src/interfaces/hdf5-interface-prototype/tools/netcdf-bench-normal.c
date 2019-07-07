// This file is part of h5-memvol.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

 
#include <mpi.h>
#include <netcdf.h>
#include <stdlib.h>

//#ifndef NC_H5VOL_MEMVOL
//#define NC_H5VOL_MEMVOL 0xC000
//#endif

// mpicc netcdf-bench.c  -l netcdf -g -std=c99

int main(int argc, char **argv) {
  int ret;
  char *file = "testfile.nc";
  int ncid;
  int dimsize = 3;
  int var, dimids[dimsize];

  MPI_Init(&argc, &argv);

  // ret = nc_create(file, NC_NETCDF4, & ncid);

  //ret = nc_create_par(file, NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, & ncid);
  ret = nc_create(file, NC_NETCDF4, &ncid);
  eassert(ret == NC_NOERR);

  ret = nc_def_dim(ncid, "lat", 100, &dimids[0]);
  eassert(ret == NC_NOERR);
  ret = nc_def_dim(ncid, "lon", 100, &dimids[1]);
  eassert(ret == NC_NOERR);
  ret = nc_def_dim(ncid, "time", NC_UNLIMITED, &dimids[2]);
  eassert(ret == NC_NOERR);

  ret = nc_def_var(ncid, "var1", NC_INT, dimsize, dimids, &var);
  eassert(ret == NC_NOERR);
  ret = nc_enddef(ncid);
  eassert(ret == NC_NOERR);

  //ret = nc_var_par_access(ncid, var, NC_INDEPENDENT);
  //eassert(ret == NC_NOERR);

  int *data = (int *)malloc(sizeof(int) * 100 * 100);
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      data[i + j * 100] = i + j * 100;
    }
  }

  // nc_set_var_chunk_cache (int ncid, int varid, size_t size, size_t nelems, float preemption)
  size_t countp[] = {100, 100, 1};

  for (int i = 0; i < 10; i++) {
    size_t startp[] = {0, 0, i};
    ret = nc_put_vara_int(ncid, var, startp, countp, data);
    eassert(ret == NC_NOERR);
  }

  // reading data back

  for (int i = 0; i < 10; i++) {
    size_t startp[] = {0, 0, i};
    ret = nc_get_vara_int(ncid, var, startp, countp, data);
    eassert(ret == NC_NOERR);
  }

  free(data);

  MPI_Finalize();
  return 0;
}
