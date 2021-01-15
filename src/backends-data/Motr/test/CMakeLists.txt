set(CMAKE_C_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MOTR_DIR} -iquote '.' -DM0_SRC_DIR=${MOTR_DIR} -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2 ")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

add_executable(clovis_test test.c)

add_library          (motr SHARED IMPORTED)
set_target_properties(motr PROPERTIES IMPORTED_LOCATION ${MOTR_DIR}/motr/.libs/libmotr.so)

target_link_libraries(clovis_test ${GLIB_LIBRARIES} esdm esdmclovis motr)
