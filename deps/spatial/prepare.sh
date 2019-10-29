#!/bin/bash -e
sourceDir="$(readlink -f $(dirname $0))"

if [[ ! -e libspatialindex ]] ; then
   git clone https://github.com/libspatialindex/libspatialindex.git
fi
pushd libspatialindex
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$PWD/../../install "$sourceDir/libspatialindex"
make -j 4 install
