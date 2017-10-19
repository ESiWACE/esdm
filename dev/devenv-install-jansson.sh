#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
prefix=$DIR/../install


echo "Installing Jansson to $prefix"

cd $DIR/..
mkdir -p install/download
cd install/download


# download most recent HDF5 source code
if [[ ! -e jansson ]] ; then
	echo "Downloading source code for Jansson"
	git clone https://github.com/akheron/jansson
fi

cd jansson


autoreconf -i
./configure --prefix=$prefix
make -j || exit 1
make -j install || exit 1

# run tests and display 
make check || exit 1
