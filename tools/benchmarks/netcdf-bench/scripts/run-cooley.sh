#!/bin/bash

NN=1

K1="mlx5_0.1/260535867176216.5:eb36fbf4-23f8-4b20-ad94-a1d8cbd0669e"
K2="mlx5_0.1/260535867181920.5:e43b6ac3-35ed-4e21-ad15-4d26e4ef4139"
K3="mlx5_0.1/260535867639876.4:2d41d846-02a1-440c-9971-e09f67530e28"
K4="mlx5_0.1/260535867639876.5:f2a82ffc-6573-4dea-8503-99b0100db8d3"
K5="mlx5_0.1/260535867176216.7:9fe60f9b-1de8-40a3-8d32-d55d38a9e174"
K6="mlx5_0.1/260535867181920.7:79865094-4084-4de0-a956-325ab88846cb"
K7="mlx5_0.1/260535867639876.6:eb26d55f-5cac-4c8c-9f68-3dc43c17491d"
K8="mlx5_0.1/260535867639876.7:ce4168e5-fc0e-4573-8b28-d41d6723df38"
K9="mlx5_0.1/260535867176216.8:d8c5a0b3-b0e9-44e8-8f0b-facf825a25b9"
K10="mlx5_0.1/260535867181920.8:4561dd1f-d79e-492b-bc42-8531aaa06078"
K11="mlx5_0.1/260535867639876.8:f8a1d7dd-03d5-478c-a7b7-4828c5bb3869"
K12="mlx5_0.1/260535867176216.9:b48f5672-fe56-4f03-a37b-4d4816a970dd"
K13="mlx5_0.1/260535867181920.9:d652d559-a215-40e9-b67f-b86d5c170b5a"
K14="mlx5_0.1/260535867639876.9:e58c053d-ac6b-4973-9722-cf932843fe4e"
FILE="$K1+$K2+$K3+$K4+$K5+$K6+$K7+$K8+$K9+$K10+$K11+$K12+$K13+$K14"

mkdir out

for FILE in "$K1+$K2+$K3+$K4+$K5+$K6+$K7+$K8+$K9+$K10+$K11+$K12+$K13+$K14"; do
  for CHUNK in "-d=40:800:800:800" "-d=800:800:800:40"; do
  $HOME/bull-io/mpio/mpio-xpd-format "$FILE"
  for TYP in ind coll; do
  for PPN in 1 2 4 5 8 10 ; do 
    OUT="$NN-$PPN-$CHUNK-$TYP"
    echo $OUT
    /home/kunkel/bull-io/mpio/mpio-xpd-format $FILE
    LD_PRELOAD=/home/kunkel/bull-io/mpio/libmpi-xpd-shmio.so mpiexec -np ${{$PPN * $NN}} ./benchtool -n ${NN} $CHUNK -p ${PPN} -t=$TYP -f "xpd:$FILE" -w -r > out/$OUT.xpd
    mpiexec -np ${{$PPN * $NN}} ./benchtool -n=${NN} -p=${PPN} $CHUNK -t=$TYP -f=gpfs.nc -w -r > out/$OUT.gpfs
    /home/kunkel/bull-io/mpio/mpi-xpd-copy "$FILE" from-xpd.nc
    cmp from-xpd.nc gpfs.nc
  done
  done
  done
done
 
