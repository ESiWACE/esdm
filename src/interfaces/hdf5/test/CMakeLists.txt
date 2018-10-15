project (${PROJECT_NAME})

# Copy environment to set HDF5_PLUIGN_PATH (convienience only)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/env.bash DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/_esdm.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  if(IS_DIRECTORY ${TESTFILE} )
    # skip, directories
  else()
    get_filename_component(TESTNAME_C ${TESTFILE} NAME)
    STRING(REGEX REPLACE ".c$" "" TESTNAME ${TESTNAME_C})
    add_executable(${TESTNAME} ${TESTFILE})
    target_link_libraries(${TESTNAME} h5-esdm)
    add_test(${TESTNAME} ./${TESTNAME})
  endif()
endforeach()
