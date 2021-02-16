#!/usr/bin/env python3

#from dask.distributed import Client
import xarray as xr
ds = xr.open_dataset('esdm://ncfile.nc', engine='netcdf4')
print(ds)
