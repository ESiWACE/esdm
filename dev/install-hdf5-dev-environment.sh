#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
prefix=$DIR/../install


echo "Installing HDF5 to $prefix"


if [[ ! -e vol ]] ; then
  echo "Downloading source code for HDF5 with VOL"
  svn checkout https://svn.hdfgroup.uiuc.edu/hdf5/features/vol/ || exit 1
fi

cd vol
echo "Preparing Configure"

mkdir build

./autogen.sh || exit 1
cd build
../configure --prefix=$prefix --enable-parallel --with-default-plugindir=$DIR/../build/ --enable-build-mode=debug --enable-hl   CFLAGS="-g" || exit 1
make -j 8 || exit 1
make -j install

echo "To verify that HDF5 works, please run"
echo "cd vol/build"
echo "make test"
