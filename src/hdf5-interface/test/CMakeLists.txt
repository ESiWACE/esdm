project (${PROJECT_NAME})


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
