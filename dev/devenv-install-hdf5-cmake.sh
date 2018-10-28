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
	#svn checkout https://svn.hdfgroup.org/hdf5/features/vol/ || exit 1
    cp ~/vol.tar .
    tar xvf vol.tar
fi


cd vol
echo "Preparing Configure"


# build H5Edefin.h and other headers
#   Cannot find source file:    <PATH>/esdm/install/download/vol/src/H5Edefin.h
./bin/make_err

rm -rf build
mkdir build
cd build

export CC=mpicc

cmake -DCMAKE_INSTALL_PREFIX:PATH=$prefix -DCMAKE_C_COMPILER:FILEPATH=gcc -DHDF5_GENERATE_HEADERS=ON -DHDF5_ENABLE_PARALLEL=ON -DHDF5_BUILD_CPP_LIB=OFF ..


make -j || exit 1
make -j install

echo "To verify that HDF5 works, please run"
echo "cd vol/build"
echo "make test"
