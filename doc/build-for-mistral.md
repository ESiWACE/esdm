# Installation Instructions for Mistral

Mistral (HLRE-3) is the supercomputer installed at DKRZ in 2015/2016.
This guide documents installation procedures to build prerequisites as well as
the prototype code base for development purposes.


## Satisfying Requirements

Mistral offers users a number of software packages via the module command (man module).
While large parts of ESDM will build with the standard tools available on the 
platform, it is still recommended to use Spack [link https://spack.readthedocs.io/en/latest/index.html]
to build, manage and install dependencies.

\code
# Download and enable Spack
git clone --depth=2 https://github.com/spack/spack.git spack
. spack/share/spack/setup-env.sh

# Set a gcc version to be used
gccver=7.3.0

# Install Dependencies
spack install gcc@$gccver +binutils
spack install autoconf%gcc@$gccver
spack install openmpi%gcc@$gccver gettext%gcc@$gccver
spack install jansson%gcc@$gccver glib%gcc@$gccver


# Load Dependencies
spack load gcc@$gccver
spack load -r autoconf%gcc@$gccver
spack load -r libtool%gcc@$gccver

spack load -r openmpi%gcc@$gccver
spack load -r jansson%gcc@$gccver
spack load -r glib%gcc@$gccver
\endcode



### HDF5 with Virtual Object Layer Support

Assuming all prerequisites have been installed and tested a development branch
of HDF5 can be build as follows.

\code
# Ensure environment is aware of dependencies installed using spack and dev-env

# Download HDF5 with VOL support
svn checkout https://svn.hdfgroup.org/hdf5/features/vol/

# Configure and build HDF5
cd vol
./autogen.sh
export CC=mpicc			# parallel features require mpicc
../configure --prefix=$prefix --enable-parallel --enable-build-mode=debug --enable-hl --enable-shared

make -j

make test

make install
\endcode

### NetCDF with Support for ESDM VOL Plugin

Assuming all prerequisites have been installed and tested a patched version of
NetCDF to enable/disable ESDM VOL Plugin support can be build as follows.

\code
# Ensure environment is aware of dependencies installed using spack and dev-env

# Downlaod NetCDF source
version=4.5.0
wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-$version.tar.gz
cd netcdf-$version

# Apply patches to allow enabling/disabling plugin
patch -b --verbose $PWD/libsrc4/nc4file.c $ESDM_REPO_ROOT/dev/netcdf4-libsrc4-nc4file-c.patch
patch -b --verbose $PWD/include/netcdf.h $ESDM_REPO_ROOT/dev/netcdf4-include-netcdf-h.patch

# Configure and build 
export CC=mpicc
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=$prefix -DCMAKE_INSTALL_PREFIX:PATH=$prefix -DCMAKE_C_FLAGS=-L$prefix/lib/ -DENABLE_PARALLEL4=ON ..

make -j

make test

make install
\endcode

## Building ESDM Prototype

Assuming all prerequisites have been installed and tested ESDM can be configured
and build as follows.

\code
# Ensure environment is aware of dependencies installed using spack and dev-env
cd $ESDM_REPO_ROOT

# Configure and build ESDM
./configure --enable-hdf5 --enable-netcdf4 --debug
cd build
make

make test
\endcode
