#!/bin/bash

FROM="$1"
TO="$2"

if [[ "$2" == "" ]] ; then
    echo "Synopsis: $0 <FROM> <TO>"
    echo "This tool renames <FROM> to <TO>"
    exit 1
fi

sed -i "s#$FROM#$TO#g" $(find -regex '.*\.[ch]\([.].*\|$\)' |grep -v build|grep -v deps)
