### H5-MEMVOL
An in-memory VOL plugin for HDF5.



### Directory structure
- dev contains stuff for development purpose
  To install hdf5-vol required by this project, call "cd dev ; ./install-dev-environment.sh"
- src contains the source code...
  To build the project call:
    cd src/
    ./configure --with-hdf5=$PWD/../install
    cd build
    make -j
  To run the tests call:
    make test
