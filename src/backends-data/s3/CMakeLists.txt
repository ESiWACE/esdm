
add_library(esdms3 SHARED s3.c ../generic-perf-model/lat-thr.c)
target_link_libraries(esdms3 esdmutils ${S3_LIBRARY})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${S3_INCLUDE_DIRS})

install(TARGETS esdms3 LIBRARY DESTINATION lib)
