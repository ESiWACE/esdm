
add_library(esdmkdsa SHARED kdsa.c ../generic-perf-model/lat-thr.c)
target_link_libraries(esdmkdsa esdmutils ${GLIB_LIBRARIES})
target_include_directories(esdmkdsa PUBLIC ${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

install(TARGETS esdmkdsa LIBRARY DESTINATION lib)
