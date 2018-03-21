
add_library(esdmposix SHARED posix.c)
target_link_libraries(esdmposix esdmutils ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

install(TARGETS esdmposix LIBRARY DESTINATION lib)
