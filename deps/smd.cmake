set(SMD_PREFIX smd-lib)
set(SMD_URL https://github.com/JulianKunkel/smd/archive/master.zip)
# set(SMD_URL_MD5 xxxxxxxx)
set(SMD_MAKE "make")

ExternalProject_Add(${SMD_PREFIX}
	PREFIX     ${SMD_PREFIX}
	URL        ${SMD_URL}
	#URL_MD5   ${SMD_URL_MD5}
	CONFIGURE_COMMAND ""
	BUILD_COMMAND  ${SMD_MAKE} -C "${CMAKE_BINARY_DIR}/deps/${SMD_PREFIX}"
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND ""
	LOG_DOWNLOAD 1
  LOG_BUILD 1
  #STEP_TARGETS ${SMD_PREFIX}_info
)

ExternalProject_Get_Property(${SMD_PREFIX} SOURCE_DIR)
message(STATUS "Source directory of ${SMD_PREFIX} ${SOURCE_DIR}")

# build another dependency
# ExternalProject_Add_Step(${SMD_PREFIX} ${SMD_PREFIX}_info
# 	COMMAND ${SMD_MAKE} info smd_build_prefix=${SMD_PREFIX}
# 	DEPENDEES build
# 	WORKING_DIRECTORY /tmp
# 	LOG 1
# )

set(SMD_DEBUG_DIR ${SOURCE_DIR}/build/${SMD_PREFIX}_debug)
set(SMD_RELEASE_DIR ${SOURCE_DIR}/build/${SMD_PREFIX}_release)

# link the correct SMD directory when the project is in Debug or Release mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	# in Debug mode
	link_directories(${SMD_RELEASE_DIR})
	set(SMD_LIBS smd_debug)
	set(SMD_LIBRARY_DIRS ${SMD_DEBUG_DIR})
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
	# in Release mode
	link_directories(${SMD_RELEASE_DIR})
	set(SMD_LIBS smd)
	set(SMD_LIBRARY_DIRS ${SMD_RELEASE_DIR})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

set(SMD_INCLUDE_DIRS ${SOURCE_DIR}/include)
message(STATUS "XUL ${SMD_INCLUDE_DIRS}")

# verify that the SMD header files can be included
set(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} 	${SMD_INCLUDE_DIRS})
CHECK_INCLUDE_FILES("smd.h;smd-datatypes.h" HAVE_SMD)
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})

if (NOT HAVE_SMD)
	message(STATUS "Did not build SMD correctly as cannot find smd.h. Will build it.")
	set(HAVE_SMD 1)
endif (NOT HAVE_SMD)
