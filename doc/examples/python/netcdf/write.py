#!/usr/bin/env python3

from netCDF4 import Dataset
import numpy as np

root_grp = Dataset('esdm://ncfile.nc', 'w', format='NETCDF4')
#root_grp = Dataset('py_netcdf4.nc', 'w')
root_grp.description = 'Example simulation data'

ndim = 128 # Size of the matrix ndim*ndim
xdimension = 0.75
ydimension = 0.75

# dimensions
root_grp.createDimension('time', None)
root_grp.createDimension('x', ndim)
root_grp.createDimension('y', ndim)

#variables
time = root_grp.createVariable('time', 'f8', ('time',))
x = root_grp.createVariable('x', 'f4', ('x',))
y = root_grp.createVariable('y', 'f4', ('y',))
field = root_grp.createVariable('field', 'f8', ('time', 'x', 'y',))

#data
x_range =  np.linspace(0, xdimension, ndim)
y_range =  np.linspace(0, ydimension, ndim)
x[:] = x_range
y[:] = y_range
for i in range(5):
   time[i] = i*50.0
   field[i,:,:] = np.random.uniform(size=(len(x_range), len(y_range)))

root_grp.close()
