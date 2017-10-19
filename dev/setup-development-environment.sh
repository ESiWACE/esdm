#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "This scripts installs the necessary development environment $DIR/../install"

source $DIR/setenv.bash

$DIR/devenv-install-hdf5.sh || exit 1
$DIR/devenv-install-netcdf.sh || exit 1
$DIR/devenv-install-jansson.sh || exit 1
