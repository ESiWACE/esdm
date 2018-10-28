
# Data Backends
option(INTERFACE_HDF5 "Compile interface for HDF5 support?" OFF)
if(INTERFACE_HDF5)
	message(STATUS "WITH_INTERFACE_HDF5")
	add_definitions(-DESDM_HAS_HDF5=1)
	SUBDIRS(hdf5)
endif()

option(INTERFACE_NETCDF "Compile interface for NETCDF support?" OFF)
if(INTERFACE_NETCDF)
	message(STATUS "WITH_INTERFACE_NETCDF")
	add_definitions(-DESDM_HAS_NETCDF=1)
	SUBDIRS(netcdf)
endif()

#option(INTERFACE_GRIB "Compile interface for GRIB support?" ON)
#if(INTERFACE_GRIB)
#	message(STATUS "WITH_INTERFACE_GRIB")
#	add_definitions(-DESDM_HAS_GRIB=1)
#	SUBDIRS(grib)
#endif()




