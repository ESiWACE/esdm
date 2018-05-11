#!/bin/bash
module load openmpi/1.8.4-gcc71 gcc/7.1.0 cmake betke/glib/2.48.0
git clone https://github.com/akheron/jansson
cd jansson
autoreconf -fi
./configure --prefix=$PWD/../deps
make -j install

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/deps/lib/pkgconfig


./configure --build-dir=debug --with-cc=/sw/rhel6-x64/gcc/gcc-7.1.0/bin/gcc --debug

./configure --with-cc=/sw/rhel6-x64/gcc/gcc-7.1.0/bin/gcc
cd build
make -j

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/../deps/lib
