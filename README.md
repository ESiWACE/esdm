# Earth System Data Middleware
<!-- {#mainpage} -->

The middleware for earth system data is a prototype to improve I/O performance
for earth system simulation as used in climate and weather applications.
ESDM exploits structural information exposed by workflows, applications as well
as data description formats such as HDF5 and NetCDF to
more efficiently organize metadata and data across a variety of storage backends.

[![Build Status](https://travis-ci.org/ESiWACE/esdm.svg?branch=master)](https://travis-ci.org/ESiWACE/esdm)

## Requirements

 * glib2
 * a variant of MPI (for parallel support with ESDM and HDF5)


#### Installation for Ubuntu/Debian Systems

  * apt-get install libglib2 libglib2.0-dev libjansson4 libjansson-dev

#### Installation for Fedora/CentOS/RHEL Systems

  * dnf install glib2 glib2-devel mpi jansson jansson-devel
  * dnf install mpich-3.0 mpich-3.0-devel


  * dnf install gcc-c++ gcc libtool cmake

#### Installation with Spack

  First get the new gcc:
  * spack install gcc@8.1.0
  Then install the packages with GCC:
  * spack install jansson%gcc@8.1.0 glib%gcc@8.1.0 openmpi%gcc@8.1.0 gettext%gcc@8.1.0
  Before running configure load the modules:
  * spack load jansson%gcc@8.1.0 glib%gcc@8.1.0 openmpi%gcc@8.1.0 gcc gettext%gcc@8.1.0 -r

## Development

### Directory structure

- `dev` contains helpers for development purpose
  e.g., to install hdf5-vol which is required by this project you can use

        ./dev/setup-development-environment.sh

- `src` contains the source code...
  To build the project call:

        source dev/activate-development-environment.bash
		./configure --debug
		cd build
		make -j

  To run the test suite call:

		cd build
		make test

  You may also choose to configure with a different hdf5 installation (see ./configure --help) e.g.:

		./configure --with-hdf5=$PWD/install



- `doc` contains documentation which uses doxygen to build a HTML or a PDF version:


	For the HTML Reference use the following commands:

		cd build
		doxygen
		build/doc/html/index.html

	For a PDF Reference (requries LaTeX) run:

		cd build
		doxygen
		cd /doc/latex
		make


- `tools` contains separate programs, e.g., for benchmarking HDF5.
  They should only be loosly coupled with the source code and allow to be used with the regular HDF5.
