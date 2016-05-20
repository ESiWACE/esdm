project (${PROJECT_NAME})


add_executable(netcdf-bench netcdf-bench.c)
target_link_libraries(netcdf-bench h5-memvol ${MPI_LIBRARIES} netcdf)
