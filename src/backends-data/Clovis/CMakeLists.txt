# Please set this to the Motr installed binary directory.
set(MOTR_DIR "/data/motr")
include_directories(${MOTR_DIR}/motr ${MOTR_DIR}/extra-libs/galois/include)
message(STATUS "MOTR DIR: ${MOTR_DIR}/motr ${MOTR_DIR}/extra-libs/galois/include")


set(CMAKE_C_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MOTR_DIR} -iquote '.' -DM0_SRC_DIR=${MOTR_DIR} -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2 -rdynamic")

set(CMAKE_CXX_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MOTR_DIR} -iquote '.' -DM0_SRC_DIR=${MOTR_DIR} -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2 -rdynamic")

add_library(esdmclovis SHARED clovis.c)
target_link_libraries(esdmclovis ${GLIB_LIBRARIES})
include_directories(${ESDM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${GLIB_INCLUDE_DIRS} ${Jansson_INCLUDE_DIRS} ${MOTR_DIR}/motr/.libs)

add_subdirectory(test)

install(TARGETS esdmclovis LIBRARY DESTINATION lib)
