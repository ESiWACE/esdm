


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

prefix=$DIR/../install

# Export relevant variables
export PATH=$prefix/bin:$PATH
export LD_LIBRARY_PATH=$prefix/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$prefix/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$prefix/include:$CPLUS_INCLUDE_PATH

export PKG_CONFIG_PATH=$prefix/lib/pkgconfig:$PKG_CONFIG_PATH


#export HDF5_PLUGIN_PATH=$prefix


# Change prompt for clarity
PS1="\nESDM devel \u@\h:\w\n\$ \[\]"

