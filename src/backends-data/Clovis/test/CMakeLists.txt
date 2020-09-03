set(CMAKE_C_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MERO_DIR} -iquote '.' -DM0_SRC_DIR=${MERO_DIR} -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2 ")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

add_executable(clovis_test test.c)

add_library          (mero SHARED IMPORTED)
set_target_properties(mero PROPERTIES IMPORTED_LOCATION ${MERO_DIR}/lib64/libmero.so)
add_library          (gf_complete SHARED IMPORTED)
set_target_properties(gf_complete PROPERTIES IMPORTED_LOCATION ${MERO_DIR}/lib64/libgf_complete.so)
add_library          (mero-xcode-ff2c SHARED IMPORTED)
set_target_properties(mero-xcode-ff2c PROPERTIES IMPORTED_LOCATION ${MERO_DIR}/lib64/libmero-xcode-ff2c.so)

target_link_libraries(clovis_test ${GLIB_LIBRARIES} esdm esdmclovis mero gf_complete mero-xcode-ff2c)
