cmake_minimum_required (VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

#
# PROJECT METADATA
# _____________________________________________________________________________

set(PROJECT_NAME esdm)
project(${PROJECT_NAME} C)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})


#
# USEFUL PROJECT VARIABLES
# _____________________________________________________________________________

set(DEV_DIR "${CMAKE_SOURCE_DIR}/dev")
set(DOC_DIR "${CMAKE_SOURCE_DIR}/doc")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")


# CMake useful variables
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")




#
# BUILD SETTINGS
# _____________________________________________________________________________

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -pedantic -Wall -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--allow-shlib-undefined -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/dev/symbols.txt -pedantic-errors")
 # -fvisibility=hidden -fvisibility-inlines-hidden

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--allow-shlib-undefined -pedantic-errors")

#-fvisibility=hidden -fvisibility-inlines-hidden
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")


#
# CNAKE MODULES & DEPENDENCIES
# _____________________________________________________________________________

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

find_package(PkgConfig REQUIRED)


# For documentation (optional)
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
# OTHER
# _____________________________________________________________________________






enable_testing()

add_subdirectory(deps)
add_subdirectory(src)


feature_summary(WHAT ALL)
