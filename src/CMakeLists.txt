



pkg_search_module(GLIB REQUIRED glib-2.0>=2.40)
set_package_properties(GLIB PROPERTIES
	DESCRIPTION "Implementation of common data structures, portabilitiy wrappers for standard OS features."
	URL "http://www.gtk.org"
	TYPE REQUIRED
	PURPOSE "Support for hash maps and tree data structures."
)

pkg_search_module(JANSSON REQUIRED jansson)
set_package_properties(JANSSON PROPERTIES
	DESCRIPTION "Library for encoding, decoding and manipulating JSON."
	URL "http://www.digip.org/jansson/"
	TYPE REQUIRED
	PURPOSE "Support for JSON metadata description."
)
include_directories(${JANSSON_INCLUDE_DIRS})


# Optional
find_package(HDF5)

find_package(MPI REQUIRED) # TODO this is optional




# Threading support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)







#
# Verbose debugging information for required software dependencies
# _____________________________________________________________________________

message(STATUS "GLIB_FOUND   ${GLIB_FOUND}")
message(STATUS "GLIB_LIBRARIES   ${GLIB_LIBRARIES}")
message(STATUS "GLIB_LIBRARY_DIR   ${GLIB_LIBRARY_DIR}")
message(STATUS "GLIB_LDFLAGS   ${GLIB_LDFLAGS}")
message(STATUS "GLIB_LDFLAGS_OTHER   ${GLIB_LDFLAGS_OTHER}")
message(STATUS "GLIB_INCLUDE_DIRS   ${GLIB_INCLUDE_DIRS}")
message(STATUS "GLIB_CFLAGS   ${GLIB_CFLAGS}")
message(STATUS "GLIB_CFLAGS_OTHER   ${GLIB_CFLAGS_OTHER}")

message(STATUS "JANSSON_FOUND   ${JANSSON_FOUND}")
message(STATUS "JANSSON_LIBRARIES   ${JANSSON_LIBRARIES}")
message(STATUS "JANSSON_LIBRARY_DIR   ${JANSSON_LIBRARY_DIR}")
message(STATUS "JANSSON_LDFLAGS   ${JANSSON_LDFLAGS}")
message(STATUS "JANSSON_LDFLAGS_OTHER   ${JANSSON_LDFLAGS_OTHER}")
message(STATUS "JANSSON_INCLUDE_DIRS   ${JANSSON_INCLUDE_DIRS}")
message(STATUS "JANSSON_CFLAGS   ${JANSSON_CFLAGS}")
message(STATUS "JANSSON_CFLAGS_OTHER   ${JANSSON_CFLAGS_OTHER}")

message(STATUS "XXX   ${XXX}")
message(STATUS "XXX   ${XXX}")
message(STATUS "XXX   ${XXX}")











#
# OPTIONS
# _____________________________________________________________________________

# General Options
option(ENABLE_MPI "Compile with MPI Support?" OFF)

# Frontends
option(ENABLE_HDF5 "Compile Plugin for HDF5 Virtual Object Layer?" OFF)

#
# TARGETS
# _____________________________________________________________________________

set(ESDM_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/src/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)


add_subdirectory(backends-data)
add_subdirectory(backends-metadata)
add_subdirectory(utils)
add_subdirectory(test)


# Include directories for all target in this CMakeLists.txt
include_directories(${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${JANSSON_INCLUDE_DIRS})
if(BACKEND_CLOVIS)
    # Please set this to the MERO/Clovis installed binary directory.
    set(MERO_DIR "/mero.bin")
    include_directories(${MERO_DIR}/include ${MERO_DIR}/include/mero)
endif()

# ESDM Utils and Database Library
add_library(esdmutils SHARED esdm.c  esdm-datatypes.c  esdm-layout.c  esdm-performancemodel.c esdm-config.c  utils/debug.c utils/json.c utils/auxiliary.c)
target_link_libraries(esdmutils ${GLIB_LDFLAGS} ${JANSSON_LDFLAGS})


# ESDM Middleware Library
add_library(esdm SHARED esdm.c  esdm-scheduler.c  esdm-modules.c)
target_link_libraries(esdm ${GLIB_LDFLAGS} ${JANSSON_LDFLAGS} esdmutils esdmposix esdmmetadummy)
if(BACKEND_MONGODB)
    target_link_libraries(esdm esdmmongodb)
endif()

if(BACKEND_CLOVIS)
    add_definitions(-DESDM_HAS_CLOVIS=1)
    target_link_libraries(esdm ${GLIB_LDFLAGS} ${JANSSON_LDFLAGS} esdmutils esdmclovis esdmmetadummy)
endif()

if(BACKEND_WOS)
    add_definitions(-DESDM_HAS_WOS=1)
    target_link_libraries(esdm ${GLIB_LDFLAGS} ${JANSSON_LDFLAGS} esdmutils esdmwos esdmmetadummy)
endif()


#target_compile_definitions(esdm PRIVATE MYDEF=${MYVAR})       # TODO: replace backend/feature enablers with this mechanism
#target_include_directories(esdm PUBLIC include PRIVATE src)   # TODO: decide on main-project/sub-project structure and switch away from include_directories directive where possible
install(TARGETS esdm LIBRARY DESTINATION lib)



#TODO: if hdf5 available.. build add hdf5-interface


#if(HDF5_FOUND)
#	SUBDIRS(hdf5-interface)
#endif(HDF5_FOUND)









#
# INSTALL & SYSTEM PACKAGING (RPM, TGZ, ...)
# _____________________________________________________________________________


# System Packaging
#include(CPackConfig)
