#!/bin/bash

# Test basic S3 behavior using minio.

ROOT="$(dirname ${BASH_SOURCE[0]})"
TYPE="basic"

if [[ ! -e $ROOT/minio ]] ; then
  wget https://dl.min.io/server/minio/release/linux-amd64/minio
  mv minio $ROOT
  chmod +x $ROOT/minio
fi

export MINIO_ACCESS_KEY=accesskey
export MINIO_SECRET_KEY=secretkey

$ROOT/minio --quiet server /dev/shm &



kill -9 %1
