if(BACKEND_KDSA_DUMMY)
	message(STATUS "WITH_BACKEND_KDSA_DUMMY")

  set (KDSA_DUMMY ./dummy/kdsa-dummy.c)
  set (KDSA_INCLUDE_DIR ./dummy/)
  set (KDSA_LIBRARY "")
else()
  set (KDSA_DUMMY "")
endif()



add_library(esdmkdsa SHARED esdm-kdsa.c ../generic-perf-model/lat-thr.c ${KDSA_DUMMY})
target_link_libraries(esdmkdsa ${GLIB_LIBRARIES} ${KDSA_LIBRARY})
target_include_directories(esdmkdsa PUBLIC ${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} ${KDSA_INCLUDE_DIR})

install(TARGETS esdmkdsa LIBRARY DESTINATION lib)
