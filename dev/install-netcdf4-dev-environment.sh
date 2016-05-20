#!/bin/bash -x

echo "This scripts installs the necessary development environment in ../install"

prefix=$PWD/../install

H5DIR=$prefix

# Download and unpack NetCDF4
if [[ ! -e netcdf-4.4.0 ]] ; then
  echo "Downloading source code for NetCDF 4"
  wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.4.0.tar.gz
  tar xvf netcdf-4.4.0.tar.gz
fi

cd netcdf-4.4.0

# Patch NetCDF to use ESD middleware
# TODO: patch netcdf to use e.g. memvol plugin for hdf5 (libsrc4/nc4file.c)

# build, check and install
CC=mpicc CPPFLAGS=-I${H5DIR}/include LDFLAGS=-L${H5DIR}/lib ./configure --prefix=$prefix

make -j
make check
make install

