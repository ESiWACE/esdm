#!/bin/bash

CC=gcc
CFLAGS="-I $HOME/programs/include"
LDFLAGS="-L $HOME/programs/lib -lesdm -lsmd -g3"

$CC $CFLAGS $LDFLAGS main.c -o main
