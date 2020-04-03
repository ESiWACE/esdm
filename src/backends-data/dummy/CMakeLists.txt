
add_library(esdmdummy SHARED dummy.c ../generic-perf-model/lat-thr.c)
target_link_libraries(esdmdummy esdmutils ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

install(TARGETS esdmdummy LIBRARY DESTINATION lib)
