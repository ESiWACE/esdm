#!/bin/bash

CC="gcc -g3"
CFLAGS="-I $HOME/install/include"
LDFLAGS="-L $HOME/install/lib -lnetcdf" 

$CC $CFLAGS $LDFLAGS write.c -o write
$CC $CFLAGS $LDFLAGS read.c -o read
