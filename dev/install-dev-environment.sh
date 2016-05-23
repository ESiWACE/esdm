#!/bin/bash -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "This scripts installs the necessary development environment $DIR/../install"

$DIR/install-hdf5-dev-environment.sh || exit 1
$DIR/install-netcdf4-dev-environment.sh || exit 1
