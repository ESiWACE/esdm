#!/bin/bash -x
SCRIPT=$(readlink -f $0)
SCRIPTPATH=$(dirname $SCRIPT)

PREFIX=$HOME/install

./configure --debug --prefix=$PREFIX
cd build
make -j
make install
