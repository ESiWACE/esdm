# Earth System Data Middleware

## H5-MEMVOL
An in-memory VOL plugin for HDF5.

## Requirements

### Installation for an Ubuntu system

  * libglib2.0-dev

## Development

### Directory structure

- dev contains stuff for development purpose
  To install hdf5-vol required by this project, 

        ./dev/install-dev-environment.sh

- src contains the source code...
  To build the project call:

        source dev/setenv.bash
		./configure --debug
		cd build
		make -j

  To run the tests call:

		make test
  
  You may also choose to configure with a different hdf5 installation (see ./configure --help) e.g.:

		./configure --with-hdf5=$PWD/install

- tools contains separate programs, e.g., for benchmarking HDF5. 
  They should only be loosly coupled with the source code and allow to be used with the regular HDF5.
