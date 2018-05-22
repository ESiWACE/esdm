set(CMAKE_CXX_FLAGS "-pedantic")

add_library(esdmwosw SHARED wos_wrapper.cpp)
target_link_libraries(esdmwosw ${GLIB_LIBRARIES} ${BOOST_boost_LIBRARY_RELEASE})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${BOOST_C_INCLUDE_DIR})

install(TARGETS esdmwosw LIBRARY DESTINATION lib)

