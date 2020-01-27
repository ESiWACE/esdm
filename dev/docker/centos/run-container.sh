#!/bin/bash

echo "You may want to run"
echo "./dev/docker/centos/build.sh"
echo

OPT="-it --rm -u $(id -u):$(id -g) -v $PWD/../../../:/data/"
docker run $OPT esdm/centos
