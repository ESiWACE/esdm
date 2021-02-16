#!/usr/bin/env python
import xarray as xr
import numpy as np
import pandas as pd
ds = xr.Dataset(
   {"var1": [1,2,3,4,5,6]},
)
ds.to_netcdf(path="esdm://ncfile.nc", format='NETCDF4', engine="netcdf4")
