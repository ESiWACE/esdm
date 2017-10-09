#!/bin/bash

NC="/home/julian/Dokumente/DKRZ/wr-git/esiwace/ESD-Middleware/install/lib"
CFLAGS="-I /home/julian/Dokumente/DKRZ/wr-git/esiwace/ESD-Middleware/install/include"
LDFLAGS="-L $NC  -l netcdf -Wl,--rpath=$NC"

for T in int double float byte int64 short; do
upper=$(echo NC_$T| tr '[:lower:]' '[:upper:]')
(
mpicc $CFLAGS -DDEBUG -g3 -O0 -std=gnu11 -o benchtool-debug-$T *.c $LDFLAGS -DDATATYPE=$T -DNC_DATATYPE=$upper || exit 1
mpicc $CFLAGS -g3 -O3 -std=gnu11 -o benchtool-$T *.c $LDFLAGS -DDATATYPE=$T -DNC_DATATYPE=$upper || exit 1
) &
done

wait
echo done
