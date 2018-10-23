#!/bin/bash

# Assuming this is using the dev environment
reporoot=$(git rev-parse --show-toplevel)


cp -r $reporoot/install/download/netcdf-4.4.0/examples/C/* test
