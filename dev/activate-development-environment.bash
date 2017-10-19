
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

prefix=$DIR/../install

export PATH=$prefix/bin:$PATH
export LD_LIBRARY_PATH=$prefix/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$prefix/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$prefix/include:$CPLUS_INCLUDE_PATH


