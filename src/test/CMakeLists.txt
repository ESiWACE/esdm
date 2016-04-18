project (${PROJECT_NAME})

file(GLOB_RECURSE TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  get_filename_component(TESTNAME_C ${TESTFILE} NAME)
  STRING(REGEX REPLACE ".c$" "" TESTNAME ${TESTNAME_C})

  add_executable(test-${TESTNAME}.exe ${TESTFILE})
  target_link_libraries(test-${TESTNAME}.exe h5-memvol)

  add_test(${TESTNAME} ./test-${TESTNAME}.exe)
endforeach()
