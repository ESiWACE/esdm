
add_library(esdmmetadummy SHARED metadummy.c)
target_link_libraries(esdmmetadummy esdmutils ${GLIB_LIBRARIES})
include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})

install(TARGETS esdmmetadummy LIBRARY DESTINATION lib)
