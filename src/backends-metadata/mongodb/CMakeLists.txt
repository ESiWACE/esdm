# MongoDB dependencies
find_package (libmongoc-1.0 1.7 REQUIRED)

message ("--   mongoc found version \"${MONGOC_VERSION}\"")
message ("--   mongoc include path \"${MONGOC_INCLUDE_DIRS}\"")
message ("--   mongoc libraries \"${MONGOC_LIBRARIES}\"")


add_library(esdmmongodb SHARED mongodb.c)

target_link_libraries (esdmmongodb PRIVATE ${GLIB_LIBRARIES} ${MONGOC_LIBRARIES})
target_include_directories (esdmmongodb PRIVATE ${MONGOC_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
target_compile_definitions (esdmmongodb PRIVATE ${MONGOC_DEFINITIONS})



install(TARGETS esdmmongodb LIBRARY DESTINATION lib)
