CC=g++
CFLAGS=-g -Wall -I.
LDFLAGS=

SOURCES=random_workload.c lfs.c
OBJECTS=$(SOURCES:.c=.o)

all: workload-random workload-random-dynamic workload-random-lfs
#all: workload-random workload-sequential workload-3dcube

%.o : %.c
	${CC} -c $(CFLAGS) $< -o $@


## Built simple
workload-random-lfs: lfs.o random_workload.o
	${CC} ${CLFAGS} ${OBJECTS} -o $@ ${LDFLAGS}

workload-random: lfs.o random_workload.o
	${CC} ${CLFAGS} ${OBJECTS} -DLFS_DUMMY_OPERATION  -o $@ ${LDFLAGS}


## Built with shared library
liblfs.so:
	${CC} -fPIC ${CFLAGS} -c -Wall lfs.c -shared -o $@ 

workload-random-dynamic: liblfs.so
	${CC} random_workload.c ${CFLAGS} -o $@ -L. -llfs

run-workload-random-dynamic:
	LD_LIBRARY_PATH=. ./workload-random-dynamic


clean:
	rm -f *.o
	rm -f workload-random
	rm -f workload-random-dynamic
	rm -f workload-random-lfs
	rm -f liblfs.so
