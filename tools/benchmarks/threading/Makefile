CFLAGS=-O3
benchmark.exe: benchmark.c Makefile
	gcc -I ../../../src/test $(CFLAGS) ../../../src/test/util/test_util.c benchmark.c -g3 -o benchmark.exe $(shell pkg-config glib-2.0 --cflags --libs) 

clean: 
	rm *.exe
