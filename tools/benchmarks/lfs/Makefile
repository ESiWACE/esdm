CC=g++
CFLAGS=-g -Wall
LDFLAGS=

SOURCES=random_workload.c lfs.c
OBJECTS=$(SOURCES:.c=.o)

all: workload-random workload-random-dynamic
#all: workload-random workload-sequential workload-3dcube

%.o : %.c
	${CC} -c $(CFLAGS) $< -o $@


## Built simple
workload-random: lfs.o random_workload.o
	${CC} ${CLFAGS} ${OBJECTS} -o $@ ${LDFLAGS}


## Built with shared library
liblfs.so:
	${CC} -fPIC -g -c -Wall lfs.c -o lfs_PIC.o
	${CC} -shared -o $@ lfs_PIC.o

workload-random-dynamic: liblfs.so
	${CC} random_workload.c -o $@ -L. -llfs

run-workload-random-dynamic:
	LD_LIBRARY_PATH=. ./workload-random-dynamic


clean:
	rm -f *.o
	rm -f workload-random
	rm -f workload-random-dynamic
	rm -f liblfs.so
