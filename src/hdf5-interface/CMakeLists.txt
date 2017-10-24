



find_package(HDF5 REQUIRED)




set(CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${HDF5_LIBRARIES} )


# Check for HDF5 with VOL support
CHECK_C_SOURCE_COMPILES("
  #include <hdf5.h>
  const H5VL_class_t H5VL_memvol;
  int main(){
    return 0;
  }
  " HDF5_VOL_TEST)

if( ${HDF5_VOL_TEST} )
else()
	message( FATAL_ERROR "The discovered HDF5 library does not support VOL interface. Please double check your HDF5 installation. Alternatively, consider using the tools to setup the development environment. A script called devenv-install-hdf5.sh can be found in the REPO_ROOT/dev directory, which compiles and installs HDF5 with VOL to your development environment." )
	#INPUT = @CMAKE_CURRENT_SOURCE_DIR@/ @CMAKE_CURRENT_SOURCE_DIR@/ @CMAKE_CURRENT_SOURCE_DIR@/../README.md  
endif()
