#!/bin/bash

if [[ ! -e ior ]]; then
git clone https://github.com/LLNL/ior
fi

DIR=$PWD/../../../install/

pushd ior
./bootstrap
./configure CFLAGS="-g -I $DIR/include" LDFLAGS="-L $DIR/lib -Wl,--rpath=$DIR/lib" --prefix=$DIR --with-hdf5 || exit 1

echo "Applying patch"
git checkout .
patch -p 1 < ../vol-selector.patch || exit 1

make -j || exit 1
# make install

popd

mv ior/src/ior ior.exe

echo "To use it set the environment variable VOL_DRIVER"
