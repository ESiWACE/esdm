#!/bin/bash

./dev/prepare-ime.sh build/centos

./configure --build-dir=build/centos --with-mpicc=/usr/lib64/mpich/bin/mpicc --with-ime-include=/data/build/centos/ime-posix/ --with-ime-lib=/data/build/centos/ime-posix/

cd build/centos
make -j 4
make test
