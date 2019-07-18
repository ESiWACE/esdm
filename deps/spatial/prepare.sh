#!/bin/bash -e
if [[ ! -e libspatialindex ]] ; then
   git clone https://github.com/libspatialindex/libspatialindex.git
fi
pushd libspatialindex
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$PWD/../../install
make -j 4 install
