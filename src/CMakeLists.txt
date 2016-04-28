cmake_minimum_required (VERSION 3.0)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_NAME h5-memvol)
project (${PROJECT_NAME})

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

set(DEV_DIR "${CMAKE_SOURCE_DIR}/../dev")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/../deps")

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

find_package(PkgConfig REQUIRED)
# pkg_search_module(HDF5 REQUIRED hdf5)

find_package(HDF5 REQUIRED)
find_package(MPI REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0>=2.40)
set_package_properties(GLIB PROPERTIES URL "http://www.gtk.org" TYPE REQUIRED PURPOSE "data structures")


### MESSAGE(STATUS "Running platform tests")

set(CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES "hdf5")

CHECK_C_SOURCE_COMPILES("
  #include <hdf5.h>
  const H5VL_class_t H5VL_memvol = {};
  int main(){
    return 0;
  }
  " HDF5_VOL_TEST)

if( ${HDF5_VOL_TEST} )
else()
    message( FATAL_ERROR "HDF5 library does not support VOL interface. Please check dev/install-dev-environment.sh" )
endif()
####


# compile flags
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/../dev/symbols.txt -pedantic-errors")
 # -fvisibility=hidden -fvisibility-inlines-hidden

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined -pedantic-errors")
#-fvisibility=hidden -fvisibility-inlines-hidden
# set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

set (CMAKE_C_FLAGS_DEBUG   "-O0 -g")
set (CMAKE_C_FLAGS_RELEASE "-O3")

## Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    find_package( Java )
    # add_feature_info( Java DESCRIPTION "Java is needed in the documentation to run Plantuml" TYPE OPTIONAL)
    if(JAVA_FOUND)
      find_path (PLANTUML_JAR_PATH plantuml.jar HINTS ${DEPS_DIR} . ${DEV_DIR} DOC "PLANTUML directory" ENV PLANTUML_JAR_PATH)
      if(PLANTUML_JAR_PATH)
        set(PLANTUML_JAR_PATH "${PLANTUML_JAR_PATH}/plantuml.jar")
        set(PLANTUML_ON ON)
      else()
        message(STATUS "Warning: no plantuml.jar found.")
      endif()
    endif(JAVA_FOUND)
    configure_file(${DEV_DIR}/Doxyfile.in Doxyfile @ONLY)
endif(DOXYGEN_FOUND)

option(plantuml "Plantuml support" ${PLANTUML_ON})
add_feature_info(plantuml plantuml  "PlantUML for documentation" URL http://de.plantuml.com/ TYPE OPTIONAL)
##### END Documentation


add_library(h5-memvol SHARED memvol.c)
target_link_libraries(h5-memvol ${HDF5_LIBRARIES} ${GLIB_LIBRARIES})
link_directories(h5-memvol ${HDF5_LIBRARY_DIRS} ${MPI_LIBRARIES} ${GLIB_LIBRARIES})
include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_BINARY_DIR} ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_PATH} ${GLIB_INCLUDE_DIRS})

SUBDIRS (test)

feature_summary(WHAT ALL)

## Installation
install(TARGETS h5-memvol LIBRARY DESTINATION lib)
