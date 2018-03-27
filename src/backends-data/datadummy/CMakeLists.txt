
add_library(esdmdatadummy SHARED datadummy.c)
target_link_libraries(esdmdatadummy ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

install(TARGETS esdmdatadummy LIBRARY DESTINATION lib)
