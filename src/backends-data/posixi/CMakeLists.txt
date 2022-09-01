
add_library(esdmposixi SHARED posixi.c ../generic-perf-model/lat-thr.c)
target_link_libraries(esdmposixi ${GLIB_LDFLAGS} ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

install(TARGETS esdmposixi LIBRARY DESTINATION lib)

SUBDIRS(test)
