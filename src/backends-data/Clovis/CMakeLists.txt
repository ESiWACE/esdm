# Please set this to the MERO/Clovis installed binary directory.
set(MERO_DIR "/work/MERO_BIN/usr")
include_directories(${MERO_DIR}/include ${MERO_DIR}/include/mero)
message(STATUS "MERO DIR: ${MERO_DIR}/include ${MERO_DIR}/include/mero")


set(CMAKE_C_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MERO_DIR} -iquote '.' -DM0_SRC_DIR=${MERO_DIR} -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2 -rdynamic")

set(CMAKE_CXX_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MERO_DIR} -iquote '.' -DM0_SRC_DIR=${MERO_DIR} -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2 -rdynamic")

add_library(esdmclovis SHARED clovis.c)
target_link_libraries(esdmclovis ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} ${MERO_DIR}/include/ ${MERO_DIR}/include/mero/)

add_subdirectory(test)

install(TARGETS esdmclovis LIBRARY DESTINATION lib)
