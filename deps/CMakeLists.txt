# https://www.selectiveintellect.net/blog/2016/7/29/using-cmake-to-add-third-party-libraries-to-your-project-1

#include(CheckIncludeFiles)
#include(ExternalProject)
#include(smd.cmake)

add_subdirectory(smd)

SET(SMD_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/smd/include CACHE PATH "SMD INCLUDES")
