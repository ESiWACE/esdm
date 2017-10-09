project (${PROJECT_NAME})


add_executable(netcdf-bench netcdf-bench.c)
target_link_libraries(netcdf-bench h5-memvol ${MPI_LIBRARIES} netcdf)

add_executable(netcdf-bench-normal netcdf-bench-normal.c)
target_link_libraries(netcdf-bench-normal h5-memvol ${MPI_LIBRARIES} netcdf)

add_executable(netcdf-bench-shm netcdf-bench-shm.c)
target_link_libraries(netcdf-bench-shm h5-memvol ${MPI_LIBRARIES} netcdf)
