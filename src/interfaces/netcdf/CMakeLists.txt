#
# CNAKE MODULES & DEPENDENCIES
# _____________________________________________________________________________ 

find_package(HDF5 REQUIRED)
find_package(MPI REQUIRED)


set(CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${HDF5_LIBRARIES})



#
# TARGETS
# _____________________________________________________________________________ 

#add_library(netcdf4-esdm SHARED netcdf4-esdm.c)
#target_link_libraries(netcdf4-esdm ${HDF5_LIBRARIES} ${GLIB_LIBRARIES} esdm)
#link_directories(netcdf4-esdm ${HDF5_LIBRARY_DIRS} ${MPI_LIBRARIES} ${GLIB_LIBRARIES})
#include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_BINARY_DIR} ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_PATH} ${GLIB_INCLUDE_DIRS})
#

#
# Testing
# _____________________________________________________________________________ 

#SUBDIRS (test)
#SUBDIRS (test-examples)
SUBDIRS (test-netcdfbench)
#SUBDIRS (test-testsuite)


