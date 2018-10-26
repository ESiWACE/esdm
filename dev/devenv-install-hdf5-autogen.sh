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

# build configure script
./autogen.sh || exit 1


# on mistral
#module load libtool
#cp `which libtool` .



# actually configure and build
rm -rf build
mkdir build
cd build

export CC=mpicc
../configure --prefix=$prefix --with-default-plugindir=$DIR/../build/ --enable-parallel --enable-build-mode=debug --enable-hl --enable-shared   CFLAGS="-g" || exit 1

make -j 8 || exit 1
make -j install

echo "To verify that HDF5 works, please run"
echo "cd vol/build"
echo "make test"
