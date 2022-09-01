#!/bin/bash

mkfs.esdm -g -l --create  --remove --ignore-errors
cdo -f nc -copy data.nc esdm://data.nc 
