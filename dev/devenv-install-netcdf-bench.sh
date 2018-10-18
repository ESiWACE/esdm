#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
prefix=$DIR/../install


NAME="netcdf-bench"


echo "Installing $NAME to $prefix"

cd $DIR/..
mkdir -p install/download
cd install/download


# download most recent HDF5 source code
if [[ ! -e netcdf-bench ]] ; then
	echo "Downloading source code for $NAME"
	git clone https://github.com/joobog/netcdf-bench
fi

cd netcdf-bench


exit 1


mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$prefix ..

make
make install


# run tests and display 
make check || exit 1
