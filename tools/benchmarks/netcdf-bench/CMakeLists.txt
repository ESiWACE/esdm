cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(NetCDF-Bench)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/dev/cmake/modules/")

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

set (CMAKE_C_FLAGS_DEBUG   "-O0 -g3 -DDEBUG")
set (CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_definitions("-DDATATYPE=int")
add_definitions("-DNC_DATATYPE=NC_INT")

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

#find_package(NetCDF REQUIRED)

find_package(PkgConfig)
pkg_search_module (NetCDF netcdf)

enable_testing()

add_subdirectory(src)




