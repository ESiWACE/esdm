
# Data Backends
option(INTERFACE_HDF5 "Compile interface for HDF5 support?" OFF)
if(INTERFACE_HDF5)
	message(STATUS "WITH_INTERFACE_HDF5")
	add_definitions(-DESDM_HAS_HDF5=1)
	SUBDIRS(hdf5)
endif()

option(INTERFACE_NETCDF4 "Compile interface for NETCDF support?" OFF)
if(INTERFACE_NETCDF4)
	message(STATUS "WITH_INTERFACE_NETCDF4")
	add_definitions(-DESDM_HAS_NETCDF=1)
	SUBDIRS(netcdf)
endif()

#option(INTERFACE_MPI "Compile interface for MPI support?" ON)
#if(INTERFACE_MPI)
#	message(STATUS "WITH_INTERFACE_MPI")
#	add_definitions(-DINTERFACE_MPI=1)
	SUBDIRS(mpi)
#endif()
