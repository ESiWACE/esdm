#!/bin/bash

FROM="$1"
TO="$2"

sed -i "s/$FROM/$TO/" *.c* *.h* algo/* test/*

