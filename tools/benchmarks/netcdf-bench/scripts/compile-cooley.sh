#!/bin/bash

NC="$HOME/install"
CFLAGS="-I $NC/include"
LDFLAGS="-L $NC/lib  -l netcdf -Wl,--rpath=$NC/lib"

mpicc $CFLAGS -DDEBUG -g3 -O0 -std=gnu99 -o benchtool *.c $LDFLAGS
