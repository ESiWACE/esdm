add_library(esdm-mdposix SHARED md-posix.c)
target_link_libraries(esdm-mdposix esdmutils ${GLIB_LIBRARIES})
include_directories(${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})

SUBDIRS(test)

install(TARGETS esdm-mdposix LIBRARY DESTINATION lib)
