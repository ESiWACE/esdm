#!/bin/zsh
setopt SH_WORD_SPLIT

SCRIPT="$(readlink -f $0)"
SCRIPTPATH="$(dirname $SCRIPT)"
cd $SCRIPTPATH

module purge
module load intel/16.0.3 
module load fca/2.5.2393
module load mxm/3.3.3002
module load bullxmpi_mlx_mt/bullxmpi_mlx_mt-1.2.8.3
module load betke/hdf5/1.8.17
module load betke/netcdf/4.4.0

module list

CC="$(which mpicc)"
CFLAGS="-g3 -O0 -std=gnu11 $(pkg-config --cflags netcdf)"
CPPFLAGS="-DDEBUG -DDATATYPE=int -DNC_DATATYPE=NC_INT"
LDFLAGS="$(pkg-config --libs netcdf)"

set -x
${CC} ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} ../../benchmark.c ../../timer.c test_benchmark.c -o run
set +x

exit 0


