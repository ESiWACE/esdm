project (${PROJECT_NAME})


file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  if(IS_DIRECTORY ${TESTFILE} )
    #message(STATUS ${TESTFILE})
  else()
    get_filename_component(TESTNAME_C ${TESTFILE} NAME)
    STRING(REGEX REPLACE ".c$" "" TESTNAME ${TESTNAME_C})

    add_executable(test-${TESTNAME}.exe ${TESTFILE})
    target_link_libraries(test-${TESTNAME}.exe h5-memvol hdf5)
  add_test(${TESTNAME} ./test-${TESTNAME}.exe)
  endif()  
endforeach()
