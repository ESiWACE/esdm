#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
prefix=$DIR/../install


echo "Installing HDF5 to $prefix"

cd $DIR/..
mkdir -p install/download
cd install/download


# download most recent HDF5 source code
if [[ ! -e vol ]] ; then
	echo "Downloading source code for HDF5 with VOL"
	svn checkout https://svn.hdfgroup.org/hdf5/features/vol/ || exit 1
fi


cd vol
echo "Preparing Configure"


# build H5Edefin.h and other headers
#   Cannot find source file:    <PATH>/esdm/install/download/vol/src/H5Edefin.h
./bin/make_err


mkdir build
cd build

cmake -DCMAKE_C_COMPILER:FILEPATH=gcc --enable-parallel ..

#../configure --prefix=$prefix --enable-parallel --with-default-plugindir=$DIR/../build/ --enable-build-mode=debug --enable-hl   CFLAGS="-g" || exit 1
#../configure --prefix=$prefix --with-default-plugindir=$DIR/../build/ --enable-build-mode=debug --enable-hl   CFLAGS="-g" || exit 1
make -j 8 || exit 1
make -j install

echo "To verify that HDF5 works, please run"
echo "cd vol/build"
echo "make test"
