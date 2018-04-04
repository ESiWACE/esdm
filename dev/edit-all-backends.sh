#!/bin/bash

# Backend Plugins to edit / Files to open
data=(posix/posix.c Clovis/clovis.c WOS/wos.c)
meta=(metadummy/metadummy.c mongodb/mongodb.c)


# Hopefully no need to change the following ###################################
cd ../src
paths=()

for i in ${data[@]}; do
	echo $i
	paths+=("backends-data/$i")
done

for i in ${meta[@]}; do
	echo $i
	paths+=("backends-metadata/$i")
done


$EDITOR -p ${paths[*]}
