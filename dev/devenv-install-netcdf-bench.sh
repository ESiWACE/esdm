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
	#git clone https://github.com/joobog/netcdf-bench
	cp -r /home/pq/ESiWACE/git/netcdf-bench .
fi

cd netcdf-bench


rm -rf build
mkdir build
cd build

#echo cmake -DCMAKE_INSTALL_PREFIX:PATH=$prefix -DCMAKE_C_FLAGS=-L$prefix/lib/  ..
cmake -DCMAKE_INSTALL_PREFIX:PATH=$prefix ..
#cmake -DCMAKE_INSTALL_PREFIX:PATH=$prefix -DCMAKE_C_FLAGS=-L$prefix/lib/  ..


make
make install


