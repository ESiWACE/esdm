add_executable(mkfs.esdm mkfs.esdm.c option.c)
target_link_libraries(mkfs.esdm esdm ${MPI_LIBRARIES} -lrt)
target_include_directories(mkfs.esdm SYSTEM PRIVATE ${MPI_INCLUDE_PATH} ${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
install(TARGETS mkfs.esdm RUNTIME DESTINATION bin)

add_executable(esdm-rm esdm-rm.c option.c tool-util.c)
target_link_libraries(esdm-rm esdm ${MPI_LIBRARIES} -lrt)
target_include_directories(esdm-rm SYSTEM PRIVATE ${MPI_INCLUDE_PATH} ${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
install(TARGETS esdm-rm RUNTIME DESTINATION bin)
