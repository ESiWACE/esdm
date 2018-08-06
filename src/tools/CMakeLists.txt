add_executable(mkfs.esdm mkfs.esdm.c option.c)
target_link_libraries(mkfs.esdm esdm ${MPI_LIBRARIES} -lrt)
target_include_directories(mkfs.esdm SYSTEM PRIVATE ${MPI_INCLUDE_PATH} ${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
