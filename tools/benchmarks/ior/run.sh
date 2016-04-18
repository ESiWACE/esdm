#!/bin/bash


export VOL_DRIVER="h5-memvol"
mpiexec -n 2 ./ior.exe -s 10 -i 1 -t 2000000 -b 2000000 -w -r -f h5.conf
