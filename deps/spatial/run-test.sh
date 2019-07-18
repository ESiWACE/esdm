#!/bin/bash -e
gcc -o test-spatial test-spatial.c -I ../../install/include/ -L ../../install/lib/ -Wl,-rpath=$PWD/../../install/lib/  -l spatialindex_c -l spatialindex -Wl,--enable-new-dtags -g3
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/../../install/lib/
./test-spatial
