#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

int main() {
  int ncid, varid;
  int data_in[6][12];
  int x, y, retval;

  nc_open("esdm://data.nc", NC_NOWRITE, &ncid);
  nc_inq_varid(ncid, "var1", &varid);
  nc_get_var_int(ncid, varid, &data_in[0][0]);

  // do something with data ...

  nc_close(ncid);
  return 0;
}

