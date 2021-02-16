#include <netcdf.h>

#define NDIMS 2
#define NX 6
#define NY 12

int main() {
  int ncid, x_dimid, y_dimid, varid;
  int dimids[NDIMS];
  int data_out[NX][NY];
  int x, y, retval;

  for (x = 0; x < NX; x++)
    for (y = 0; y < NY; y++)
      data_out[x][y] = x * NY + y;

  nc_create("esdm://data.nc", NC_CLOBBER, &ncid);
  nc_def_dim(ncid, "x", NX, &x_dimid);
  nc_def_dim(ncid, "y", NY, &y_dimid);

  dimids[0] = x_dimid;
  dimids[1] = y_dimid;

  nc_def_var(ncid, "var1", NC_INT, NDIMS, dimids, &varid);
  nc_enddef(ncid);
  nc_put_var_int(ncid, varid, &data_out[0][0]);
  nc_close(ncid);
  return 0;
}

