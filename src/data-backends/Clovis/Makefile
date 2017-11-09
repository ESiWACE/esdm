MERO_DIR="/mero"
GF_DIR = ${MERO_DIR}/extra-libs/gf-complete

AM_CPPFLAGS = -D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -iquote ${MERO_DIR} -iquote '.' -include 'config.h' -DM0_SRC_DIR=${MERO_DIR} -I${GF_DIR}/include -I/usr/src/lustre-2.5.1-headers/libcfs/include -I/usr/src/lustre-2.5.1-headers/lnet/include -I/usr/src/lustre-2.5.1-headers/lustre/include -I../../include
AM_CFLAGS = -fno-common -Wall -Werror -Wno-attributes -fno-strict-aliasing -fno-omit-frame-pointer -g -O2
AM_LDFLAGS = -rdynamic -pthread -Wl,-rpath -Wl,/usr/local/lib
AM_LIBS = ${MERO_DIR}/mero/.libs/libmero.so -lm -lpthread -laio -lrt -lyaml -luuid ${GF_DIR}/src/.libs/libgf_complete.so -ldl ${MERO_DIR}/ut/.libs/libmero-ut.so ${MERO_DIR}/net/test/.libs/libmero-net-test.so ${MERO_DIR}/xcode/ff2c/.libs/libmero-xcode-ff2c.so

HDR = clovis.h
SRC = clovis.c
OBJ = clovis.o
TGT = clovis_test

all: ${TGT}
.PHONY: all

${OBJ}:${SRC} 
	gcc ${AM_CPPFLAGS} ${AM_CFLAGS} -c -o ${OBJ} ${SRC}

test.o: test/test.c
	gcc ${AM_CPPFLAGS} ${AM_CFLAGS} -c -o test.o test/test.c

${TGT}:${OBJ} test.o
	gcc ${AM_CFLAGS} ${AM_LDFLAGS} -o ${TGT} ${OBJ} test.o ${AM_LIBS}

clean:
	rm -f ${OBJ} ${TGT} test.o
