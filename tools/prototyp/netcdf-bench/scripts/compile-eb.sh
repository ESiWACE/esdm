#!/bin/zsh
setopt SH_WORD_SPLIT

SCRIPT="$(readlink -f $0)"
SCRIPTPATH="$(dirname $SCRIPT)"
cd "$SCRIPTPATH/.."

module purge
module load gcc/5.1.0
#module load intel/17.0.1
#module load fca/2.5.2431
#module load mxm/3.4.3082 
#module load bullxmpi_mlx_mt/bullxmpi_mlx_mt-1.2.9.2
module load openmpi/1.8.4-gcc51

module load betke/cmake
module load betke/hdf5/1.8.17
module load betke/netcdf/4.4.0

#module load betke/glibc
#module load betke/glib
#module load betke/boost

module list

[ ! -d build ] && mkdir build
cd build



PREFIX="$SCRIPTPATH/../install"
NCDIR="/work/ku0598/k202107/software/install/netcdf/4.4.0"

#CMAKEARGS="-DCMAKE_C_COMPILER=$(which icc)"
CMAKEARGS="-DCMAKE_C_COMPILER=$(which gcc)"
CMAKEARGS="-DCMAKE_INSTALL_PREFIX=$PREFIX $CMAKEARGS"
CMAKEARGS="-DNETCDF_INCLUDE_DIR=$NCDIR/include $CMAKEARGS"
CMAKEARGS="-DNETCDF_LIBRARY=$NCDIR/lib/libnetcdf.so $CMAKEARGS"
#CMAKEARGS="-DCMAKE_BUILD_TYPE=Debug $CMAKEARGS"
CMAKEARGS="-DCMAKE_BUILD_TYPE=Release $CMAKEARGS"



set -x
cmake $CMAKEARGS ..
set +x

cd build
make -j install
