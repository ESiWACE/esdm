project(NetCDF-Bench)

add_executable(benchtool 
	benchmark.c
	main.c
	options.c
	report.c
	timer.c
	types.c
	)

target_link_libraries(benchtool 
	${MPI_LIBRARIES}
	${NETCDF_LIBRARIES}
	rt
	m
	)

if(MPI_COMPILE_FLAGS)
  set_target_properties(benchtool PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(benchtool PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

#add_test(table CONFIGURATION Debug ./test/test_table.cpp) 

install_programs(/bin FILES benchtool)
