
add_library(esdmmetadummy SHARED metadummy.c)
target_link_libraries(esdmmetadummy esdmutils ${GLIB_LIBRARIES})
include_directories(${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})


SUBDIRS(test)


install(TARGETS esdmmetadummy LIBRARY DESTINATION lib)
