add_library(esdmpmem SHARED esdm-pmem.c ../generic-perf-model/lat-thr.c ${PMEM_DUMMY})
target_link_libraries(esdmpmem esdmutils ${GLIB_LIBRARIES} ${PMEM_LIBRARY})
target_include_directories(esdmpmem PUBLIC ${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} ${PMEM_INCLUDE_DIR})

install(TARGETS esdmpmem LIBRARY DESTINATION lib)
