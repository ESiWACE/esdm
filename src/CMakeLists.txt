pkg_search_module(GLIB REQUIRED glib-2.0>=2.40)
set_package_properties(GLIB PROPERTIES
	DESCRIPTION "Implementation of common data structures, portability wrappers for standard OS features."
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

pkg_search_module(SCIL scil)
set_package_properties(SCIL PROPERTIES
	DESCRIPTION "Library for compression."
	URL "https://github.com/JulianKunkel/scil"
	TYPE OPTIONAL
	PURPOSE "Support for SCIL compression."
)

find_package (Threads)

# Optional
find_package(MPI REQUIRED) # TODO this is optional


# Threading support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${GLIB_LIBRARY_DIRS}:${JANSSON_LIBRARY_DIRS}:${SCIL_LIBRARY_DIRS}")





#
# Verbose debugging information for required software dependencies
# _____________________________________________________________________________

message(STATUS "GLIB_FOUND   ${GLIB_FOUND}")
message(STATUS "GLIB_LIBRARIES   ${GLIB_LIBRARIES}")
message(STATUS "GLIB_LIBRARY_DIRS   ${GLIB_LIBRARY_DIRS}")
message(STATUS "GLIB_LDFLAGS   ${GLIB_LDFLAGS}")
message(STATUS "GLIB_LDFLAGS_OTHER   ${GLIB_LDFLAGS_OTHER}")
message(STATUS "GLIB_INCLUDE_DIRS   ${GLIB_INCLUDE_DIRS}")
message(STATUS "GLIB_CFLAGS   ${GLIB_CFLAGS}")
message(STATUS "GLIB_CFLAGS_OTHER   ${GLIB_CFLAGS_OTHER}")

message(STATUS "JANSSON_FOUND   ${JANSSON_FOUND}")
message(STATUS "JANSSON_LIBRARIES   ${JANSSON_LIBRARIES}")
message(STATUS "JANSSON_LIBRARY_DIRS   ${JANSSON_LIBRARY_DIRS}")
message(STATUS "JANSSON_LDFLAGS   ${JANSSON_LDFLAGS}")
message(STATUS "JANSSON_LDFLAGS_OTHER   ${JANSSON_LDFLAGS_OTHER}")
message(STATUS "JANSSON_INCLUDE_DIRS   ${JANSSON_INCLUDE_DIRS}")
message(STATUS "JANSSON_CFLAGS   ${JANSSON_CFLAGS}")
message(STATUS "JANSSON_CFLAGS_OTHER   ${JANSSON_CFLAGS_OTHER}")

if(SCIL_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_SCIL ${SCIL_CFLAGS}")
endif(SCIL_FOUND)

message(STATUS "SCIL_FOUND   ${SCIL_FOUND}")
message(STATUS "SCIL_LIBRARIES   ${SCIL_LIBRARIES}")
message(STATUS "SCIL_LIBRARY_DIRS   ${SCIL_LIBRARY_DIRS}")
message(STATUS "SCIL_LDFLAGS   ${SCIL_LDFLAGS}")
message(STATUS "SCIL_LDFLAGS_OTHER   ${SCIL_LDFLAGS_OTHER}")
message(STATUS "SCIL_INCLUDE_DIRS   ${SCIL_INCLUDE_DIRS}")
message(STATUS "SCIL_CFLAGS   ${SCIL_CFLAGS}")
message(STATUS "SCIL_CFLAGS_OTHER   ${SCIL_CFLAGS_OTHER}")








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

set(ESDM_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/src/include;${GLIB_INCLUDE_DIRS};${JANSSON_INCLUDE_DIRS};${SMD_INCLUDES}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

add_subdirectory(backends-metadata)
add_subdirectory(utils)
add_subdirectory(test)
add_subdirectory(tools)




# Include directories for all target in this CMakeLists.txt
include_directories(${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${JANSSON_INCLUDE_DIRS} ${SMD_INCLUDES} ${SCIL_INCLUDE_DIRS})


# ESDM Middleware Library
add_library(esdm SHARED esdm.c esdm-scheduler.c esdm-stream.c fragments.c esdm-modules.c backends-data/init.c estream.c esdm-attributes.c esdm-datatypes.c esdm-layout.c esdm-performancemodel.c esdm-config.c performance.c hypercube.c hypercube-neighbour-manager.c esdm-grid.c utils/debug.c utils/auxiliary.c)
target_link_libraries(esdm ${GLIB_LDFLAGS} ${JANSSON_LDFLAGS} ${SCIL_LDFLAGS} ${CMAKE_THREAD_LIBS_INIT} esdmdummy esdm-mdposix smd m)
if(BACKEND_MONGODB)
    target_link_libraries(esdm esdmmongodb)
endif()


#target_compile_definitions(esdm PRIVATE MYDEF=${MYVAR})       # TODO: replace backend/feature enablers with this mechanism
#target_include_directories(esdm PUBLIC include PRIVATE src)   # TODO: decide on main-project/sub-project structure and switch away from include_directories directive where possible

install(TARGETS esdm LIBRARY DESTINATION lib)
install(FILES "./include/esdm-datatypes.h" DESTINATION include)
install(FILES "./include/esdm.h" DESTINATION include)
install(FILES "./include/esdm-stream.h" DESTINATION include)
install(FILES "./include/esdm-mpi.h" DESTINATION include)

SUBDIRS(interfaces)



### Data backends ###
SUBDIRS(backends-data/dummy)

option(BACKEND_S3 "Compile backend for S3 support?" OFF)
if(BACKEND_S3)
	message(STATUS "WITH_BACKEND_S3")
	add_definitions(-DESDM_HAS_S3=1)
	SUBDIRS(backends-data/s3)

  find_path(S3_INCLUDE_DIR libs3.h HINTS ${S3_INCLUDE_DIR})
  find_library(S3_LIBRARY NAMES s3 HINTS ${S3_LIB_DIR})

  target_link_libraries(esdm esdms3)
endif()

option(BACKEND_POSIX "Compile backend for POSIX support?" ON)
if(BACKEND_POSIX)
	message(STATUS "WITH_BACKEND_POSIX")
	add_definitions(-DESDM_HAS_POSIX=1)
	SUBDIRS(backends-data/posix)
  target_link_libraries(esdm esdmposix)
  
	SUBDIRS(backends-data/posixi)
  target_link_libraries(esdm esdmposixi)
endif()


option(BACKEND_LUSTRE "Compile backend for Lustre support?" OFF)
if(BACKEND_LUSTRE)
	message(STATUS "WITH_BACKEND_LUSTRE")
	add_definitions(-DESDM_HAS_LUSTRE=1)
	SUBDIRS(backends-data/lustre)
endif()

option(BACKEND_MOTR "Compile backend for Motr support?" OFF)
if(BACKEND_MOTR)
	message(STATUS "WITH_BACKEND_MOTR")
	add_definitions(-DESDM_HAS_MOTR=1)
  target_link_libraries(esdm esdmmotr)
	SUBDIRS(backends-data/Motr)
endif()

option(BACKEND_WOS "Compile backend for WOS support?" OFF)
if(BACKEND_WOS)
	message(STATUS "WITH_BACKEND_WOS")
	add_definitions(-DESDM_HAS_WOS=1)
  target_link_libraries(esdm esdmwos)
	SUBDIRS(backends-data/WOS)
endif()


option(BACKEND_KDSA "Compile backend for Kove XPD / KDSA API" OFF)
option(BACKEND_KDSA_DUMMY "Compile dummy file backend for Kove XPD / KDSA API" OFF)

message(STATUS "Searching for KDSA libraries")
find_path(KDSA_INCLUDE_DIR kdsa.h HINTS ${KDSA_INCLUDE_DIR})
find_library(KDSA_LIBRARY NAMES kdsa HINTS ${KDSA_LIB_DIR})
string(COMPARE EQUAL "${KDSA_LIBRARY}" KDSA_LIBRARY-NOTFOUND _cmp)
if(_cmp)
  message("not found")
else()
  message(STATUS "found in ${KDSA_LIBRARY} and ${KDSA_INCLUDE_DIR}")
  set(BACKEND_KDSA ON)
endif()
if(BACKEND_KDSA OR BACKEND_KDSA_DUMMY)
	message(STATUS "WITH_BACKEND_KDSA")
	add_definitions(-DESDM_HAS_KDSA=1)
  target_link_libraries(esdm esdmkdsa)
	SUBDIRS(backends-data/kdsa)
endif()

message(STATUS "Searching for IME libraries")
option(BACKEND_IME "Compile backend for IME" OFF)
find_path(IME_INCLUDE_DIR ime_native.h HINTS ${IME_INCLUDE_DIR})
find_library(IME_LIBRARY NAMES im_client HINTS ${IME_LIB_DIR})
string(COMPARE EQUAL "${IME_LIBRARY}" IME_LIBRARY-NOTFOUND _cmp)
if(_cmp)
  message("not found")
else()
  message(STATUS "found in ${IME_LIBRARY} and ${IME_INCLUDE_DIR}")
  set(BACKEND_IME ON)
endif()
if(BACKEND_IME)
	message(STATUS "WITH_BACKEND_IME")
	add_definitions(-DESDM_HAS_IME=1)
  target_link_libraries(esdm esdmime ${IME_LIBRARY})
	SUBDIRS(backends-data/ime)
endif()


message(STATUS "Searching for PMEM libraries")

option(BACKEND_PMEM "Compile backend for PMEM" OFF)

find_path(PMEM_INCLUDE_DIR libpmem.h HINTS ${PMEM_INCLUDE_DIR})
find_library(PMEM_LIBRARY NAMES pmem HINTS ${PMEM_LIB_DIR})
string(COMPARE EQUAL "${PMEM_LIBRARY}" PMEM_LIBRARY-NOTFOUND _cmp)
if(_cmp)
  message("not found")
else()
  message(STATUS "found in ${PMEM_LIBRARY} and ${PMEM_INCLUDE_DIR}")
  set(BACKEND_PMEM ON)
endif()

if(BACKEND_PMEM)
	message(STATUS "WITH_BACKEND_PMEM")
	add_definitions(-DESDM_HAS_PMEM=1)

  SUBDIRS(backends-data/pmem)
  target_link_libraries(esdm esdmpmem)
endif()

### END Data backends ###


#
# INSTALL & SYSTEM PACKAGING (RPM, TGZ, ...)
# _____________________________________________________________________________


# System Packaging
#include(CPackConfig)
