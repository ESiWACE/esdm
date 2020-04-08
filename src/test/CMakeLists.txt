#
# Tests
# _____________________________________________________________________________

# Copy some prepared files such as a ESDM configuration to the build test directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/esdm.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/esdm-posix.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/esdm-wos.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/esdm-clovis.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/esdm-mixed.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cleanup.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



# Generic tests, that only require libesdm
file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")
foreach(TESTFILE ${TESTFILES})
  if(IS_DIRECTORY ${TESTFILE} )
    #message(STATUS ${TESTFILE})
  else()
    get_filename_component(TESTNAME_C ${TESTFILE} NAME)
    STRING(REGEX REPLACE ".c$" "" TESTNAME ${TESTNAME_C})

	# Build, link and add as test
    add_executable(${TESTNAME} ${TESTFILE} util/test_util.c)
   	target_link_libraries(${TESTNAME} esdm esdmmpi -lrt)
    target_include_directories(${TESTNAME} SYSTEM PRIVATE ${MPI_INCLUDE_PATH} ${CMAKE_BINARY_DIR} ${ESDM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${SMD_INCLUDES})

    add_test(${TESTNAME} ./${TESTNAME})
  endif()
endforeach()




# A special test with additional requirements
#add_executable(${TESTNAME} ${TESTFILE})
#target_link_libraries(${TESTNAME} "${PROJECT_BINARY_DIR}/libesdm.so" "libspecial.so")
#add_test(${TESTNAME} ./${TESTNAME})
