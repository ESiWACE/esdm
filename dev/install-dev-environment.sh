#!/bin/bash

echo "This scripts installs the necessary development environment in ../install"

DIR=$PWD/../

if [[ ! -e vol ]] ; then
  echo "Downloading code"
  svn checkout https://svn.hdfgroup.uiuc.edu/hdf5/features/vol/ || exit 1
fi

cd vol
mkdir build
cd build
../configure --prefix=$DIR/install --enable-parallel --with-default-plugindir=$DIR/src/build/  --enable-debug=all  --enable-hl   CFLAGS="-g" || exit 1
make -j 8 || exit 1
make -j install

echo "To verify that HDF5 works, please run"
echo "cd vol/build"
echo "make test"
