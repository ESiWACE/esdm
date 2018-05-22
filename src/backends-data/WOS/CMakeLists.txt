add_library(esdmwos SHARED wos.c ../generic-perf-model/lat-thr.c)
target_link_libraries(esdmwos ${GLIB_LIBRARIES} ${JANSSON_LDFLAGS} ${WOS_wos_LIBRARY_RELEASE} esdmwosw)
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} ${WOS_C_INCLUDE_DIR} wrapper)

add_subdirectory(wrapper)

install(TARGETS esdmwos LIBRARY DESTINATION lib)

