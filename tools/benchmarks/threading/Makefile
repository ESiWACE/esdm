CFLAGS=-O3
all: benchmark.exe benchmark-unthreaded.exe

benchmark.exe: benchmark.c Makefile
	gcc -I ../../../src/test $(CFLAGS) -std=gnu11 ../../../src/test/util/test_util.c benchmark.c -g3 -o benchmark.exe $(shell pkg-config glib-2.0 --cflags --libs)  -l pthread 

benchmark-unthreaded.exe: benchmark.c Makefile
	gcc -I ../../../src/test $(CFLAGS) -DTEST_SINGLE_THREADED -std=gnu11 ../../../src/test/util/test_util.c benchmark.c -g3 -o benchmark-unthreaded.exe $(shell pkg-config glib-2.0 --cflags --libs)  -l pthread 

clean:
	rm *.exe
