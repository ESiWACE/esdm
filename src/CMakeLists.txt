cmake_minimum_required (VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../cmake)

#
# PROJECT METADATA
# _____________________________________________________________________________ 

set(PROJECT_NAME esdm)
project(${PROJECT_NAME})

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


#
# USEFUL PROJECT VARIABLES
# _____________________________________________________________________________ 

set(DEV_DIR "${CMAKE_SOURCE_DIR}/../dev")
set(DOC_DIR "${CMAKE_SOURCE_DIR}/../doc")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/../deps")

# CMake useful variables
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")


#
# CNAKE MODULES & DEPENDENCIES
# _____________________________________________________________________________ 

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0>=2.40)
set_package_properties(GLIB PROPERTIES
	DESCRIPTION "Implementation of common data structures, portabilitiy wrappers for standard OS features."
	URL "http://www.gtk.org"
	TYPE REQUIRED
	PURPOSE "Support for hash maps and tree data structures."
)

pkg_search_module(Jansson REQUIRED jansson)
set_package_properties(Jansson PROPERTIES
	DESCRIPTION "Library for encoding, decoding and manipulating JSON."
	URL "http://www.digip.org/jansson/"
	TYPE REQUIRED
	PURPOSE "Support for JSON metadata description."
)



# Optional
find_package(HDF5)




# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    #find_package( Java )
    # add_feature_info( Java DESCRIPTION "Java is needed in the documentation to run Plantuml" TYPE OPTIONAL)
    #if(JAVA_FOUND)
      find_path (PLANTUML_JAR_PATH plantuml.jar HINTS "/usr/share/java/" ${DEPS_DIR} . ${DEV_DIR} DOC "PLANTUML directory" ENV PLANTUML_JAR_PATH)
      if(PLANTUML_JAR_PATH)
        set(PLANTUML_JAR_PATH "${PLANTUML_JAR_PATH}/plantuml.jar")
        set(PLANTUML_ON ON)
      else()
        message(STATUS "Warning: plantuml.jar not found.")
      endif()
    #endif(JAVA_FOUND)
    configure_file(${DOC_DIR}/Doxyfile.in Doxyfile @ONLY)
endif(DOXYGEN_FOUND)

option(plantuml "Plantuml support" ${PLANTUML_ON})
add_feature_info(plantuml plantuml  "PlantUML for documentation" URL http://de.plantuml.com/ TYPE OPTIONAL)


#
# BUILD SETTINGS
# _____________________________________________________________________________ 

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/../dev/symbols.txt -pedantic-errors")
 # -fvisibility=hidden -fvisibility-inlines-hidden

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined -pedantic-errors")
#-fvisibility=hidden -fvisibility-inlines-hidden
# set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

set (CMAKE_C_FLAGS_DEBUG   "-O0 -g")
set (CMAKE_C_FLAGS_RELEASE "-O3")


#
# TARGETS
# _____________________________________________________________________________ 

## Application
add_library(esdm SHARED esdm.c  esdm-datatypes.c  esdm-layout.c  esdm-performancemodel.c  esdm-scheduler.c  esdm-siteconfig.c  esdm-modules.c  utils/debug.c)
target_link_libraries(esdm ${GLIB_LIBRARIES} ${Jansson_LIBRARIES})
link_directories(esdm ${GLIB_LIBRARIES} ${Jansson_LIBRARIES})
include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

#TODO: if hdf5 available.. build add hdf5-interface



enable_testing()
add_subdirectory(data-backends)
add_subdirectory(utils)
add_subdirectory(test)

if(HDF5_FOUND)
	#SUBDIRS(hdf5-interface)
endif(HDF5_FOUND)









#
# INSTALL & SYSTEM PACKAGING (RPM, TGZ, ...)
# _____________________________________________________________________________ 

install(TARGETS esdm LIBRARY DESTINATION lib)

# System Packaging
#include(CPackConfig)


#
# OTHER
# _____________________________________________________________________________ 

feature_summary(WHAT ALL)

