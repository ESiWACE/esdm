add_library(esdmwos SHARED wos.c)
target_link_libraries(esdmwos ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} wrapper)

add_subdirectory(wrapper)

install(TARGETS esdmwos LIBRARY DESTINATION lib)

