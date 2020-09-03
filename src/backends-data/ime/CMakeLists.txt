
add_library(esdmime SHARED ime.c ../generic-perf-model/lat-thr.c)
target_link_libraries(esdmime ${GLIB_LIBRARIES} ${IME_LIBRARY})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} ${IME_INCLUDE_DIR})

install(TARGETS esdmime LIBRARY DESTINATION lib)
