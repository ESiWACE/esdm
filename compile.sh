#!/bin/bash
SCRIPT=$(readlink -f $0)
SCRIPTPATH=$(dirname $SCRIPT)

PREFIX=$HOME/install

./configure --prefix=$PREFIX
cd build
make -j
make install
