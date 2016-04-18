cmake_minimum_required (VERSION 3.2)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_NAME h5-memvol)
project (${PROJECT_NAME})

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

find_package(PkgConfig REQUIRED)
# pkg_search_module(HDF5 REQUIRED hdf5)

find_package(HDF5 REQUIRED)
find_package(MPI REQUIRED)

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)



# compile flags
set (CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -ggdb3 -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined " # -fvisibility=hidden -fvisibility-inlines-hidden
 #-pedantic-errors
)
set ( CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -ggdb3 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined " # -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/symbols.txt -fvisibility=hidden -fvisibility-inlines-hidden
)
# set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

set (CMAKE_C_FLAGS_DEBUG   "-O0")
set (CMAKE_C_FLAGS_RELEASE "-O3")

add_library(h5-memvol SHARED memvol.c)
target_link_libraries(h5-memvol ${HDF5_LIBRARIES})
link_directories(h5-memvol ${HDF5_LIBRARY_DIRS} ${MPI_LIBRARIES})

include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_BINARY_DIR} ${HDF5_INCLUDE_DIR} ${MPI_INCLUDE_PATH})

# SUBDIRS (test)

## Installation
# install(TARGETS xxx LIBRARY DESTINATION lib)
