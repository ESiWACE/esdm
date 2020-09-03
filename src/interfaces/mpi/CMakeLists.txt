add_library(esdmmpi SHARED esdm-mpi.c)
target_link_libraries(esdmmpi ${MPI_LIBRARIES})
target_include_directories(esdmmpi PRIVATE ${MPI_INCLUDE_PATH} ${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${SMD_INCLUDES})
install(TARGETS esdmmpi LIBRARY DESTINATION lib)
