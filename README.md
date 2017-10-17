# Earth System Data Middleware

The middleware for earth system data is 

## Requirements

 * glib2
 * a variant of MPI (for parallel support with ESDM and HDF5)


#### Installation for Ubuntu/Debian Systems

  * apt-get install libglib2.0-dev

#### Installation for Fedora/CentOS/RHEL Systems

  * dnf install glib2 glib2-devel mpi



## Development

### Directory structure

- `dev` contains helpers for development purpose
  e.g., to install hdf5-vol which is required by this project you can use 

        ./dev/install-dev-environment.sh

- `src` contains the source code...
  To build the project call:

        source dev/setenv.bash
		./configure --debug
		cd build
		make -j

  To run the tests call:

		make test
  
  You may also choose to configure with a different hdf5 installation (see ./configure --help) e.g.:

		./configure --with-hdf5=$PWD/install



- `doc` contains the documentation which requires doxygen to build

	
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
