# Specify the minimum version you require.
find_package (libmongoc-1.0 1.7 REQUIRED)

message ("--   mongoc found version \"${MONGOC_VERSION}\"")
message ("--   mongoc include path \"${MONGOC_INCLUDE_DIRS}\"")
message ("--   mongoc libraries \"${MONGOC_LIBRARIES}\"")

# The "hello_mongoc.c" sample program is shared among four tests.
add_executable (hello_mongoc hello_mongoc.c)
target_include_directories (hello_mongoc PRIVATE "${MONGOC_INCLUDE_DIRS}")
target_link_libraries (hello_mongoc PRIVATE "${MONGOC_LIBRARIES}")
target_compile_definitions (hello_mongoc PRIVATE "${MONGOC_DEFINITIONS}")









#add_library(esdmmongodb SHARED metadummy.c)
#target_link_libraries(esdmmongodb esdmutils ${GLIB_LIBRARIES})
#include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS})
#
#install(TARGETS esdmmongodb LIBRARY DESTINATION lib)
