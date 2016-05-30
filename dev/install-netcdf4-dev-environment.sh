#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
prefix=$DIR/../install

echo "Installing NetCDF 4 to $prefix"
cd $DIR/..
mkdir -p install/download
cd install/download

# set PATH, etc.
source $DIR/setenv.bash

# required for netCDF to link the correct HDF5 installation
H5DIR=$prefix

# Download and unpack NetCDF4
if [[ ! -e netcdf-4.4.0 ]] ; then
  echo "Downloading source code for NetCDF 4"
  wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.4.0.tar.gz || exit 1
  tar xvf netcdf-4.4.0.tar.gz || exit 1
fi

cd netcdf-4.4.0

# patch netcdf
patch -b --verbose $PWD/libsrc4/nc4file.c $DIR/netcdf4-libsrc4-nc4file-c.patch
patch -b --verbose $PWD/include/netcdf.h $DIR/netcdf4-include-netcdf-h.patch

# Patch NetCDF to use ESD middleware
# TODO: patch netcdf to use e.g. memvol plugin for hdf5 (libsrc4/nc4file.c)

# build, check and install

CC=mpicc CPPFLAGS=-I${H5DIR}/include LDFLAGS=-L${H5DIR}/lib ./configure --prefix=$prefix

make -j
make check
make install

