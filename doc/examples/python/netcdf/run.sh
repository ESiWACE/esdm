#!/bin/bash

mkfs.esdm -g -l --create  --remove --ignore-errors
./write.py
#./read.py
