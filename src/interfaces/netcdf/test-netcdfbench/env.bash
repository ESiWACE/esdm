reporoot=$(git rev-parse --show-toplevel)

export HDF5_PLUGIN_PATH=$reporoot/build/src/interfaces/hdf5/
echo $HDF5_PLUGIN_PATH

