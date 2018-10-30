#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
prefix=$DIR/../install


NAME="netcdf-bench"


echo "Installing $NAME to $prefix"

cd $DIR/..
mkdir -p install/download
cd install/download


# download source code
if [[ ! -e netcdf-bench ]] ; then
	echo "Downloading source code for $NAME"
	git clone https://github.com/joobog/netcdf-bench
	#cp -r /home/pq/ESiWACE/git/netcdf-bench .
fi

cd netcdf-bench


rm -rf build
mkdir build
cd build

cmake -DCMAKE_INSTALL_PREFIX:PATH=$prefix ..


make
make install


