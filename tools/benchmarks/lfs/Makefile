MPICC=mpicc
CC=gcc
CFLAGS=-g -Wall -I. -std=gnu99 -pedantic
LDFLAGS=

SOURCES=random_workload.c sequential_workload.c lfs.c lfs-mpi.c random_workload_mpi.c
OBJECTS=$(SOURCES:.c=.o)

all: workload-random-dummy workload-random-dynamic workload-random-lfs workload-sequential-dummy workload-sequential-lfs workload-mpi
#all: workload-random workload-sequential workload-3dcube

%.o : %.c
	@echo $?
	${MPICC} -c $(CFLAGS) $< -o $@


## Built simple
workload-mpi: lfs-mpi.o random_workload_mpi.o
	${MPICC} ${CFLAGS} $? -o $@ ${LDFLAGS}

workload-random-lfs: lfs.o random_workload.o
	${CC} ${CFLAGS} $? -o $@ ${LDFLAGS}

workload-random-dummy: random_workload.c lfs-dummy.h
	${CC} ${CFLAGS} random_workload.c -D LFS_DUMMY_OPERATION  -o $@ ${LDFLAGS}

workload-sequential-lfs: lfs.o sequential_workload.o
	${CC} ${CFLAGS} $? -o $@ ${LDFLAGS}

workload-sequential-dummy: sequential_workload.c lfs-dummy.h
	${CC} ${CFLAGS} sequential_workload.c -D LFS_DUMMY_OPERATION  -o $@ ${LDFLAGS}

## Built with shared library
liblfs.so:
	${CC} -fPIC ${CFLAGS} -c -Wall lfs.c -shared -o $@

workload-random-dynamic: liblfs.so
	${CC} random_workload.c ${CFLAGS} -o $@ -L. -llfs

run-workload-random-dynamic:
	LD_LIBRARY_PATH=. ./workload-random-dynamic


clean:
	rm -f *.o
	rm -f workload-random-dummy
	rm -f workload-mpi
	rm -f workload-random-dynamic
	rm -f workload-random-lfs
	rm -f workload-sequential-dummy
	rm -f workload-sequential-lfs
	rm -f liblfs.so
