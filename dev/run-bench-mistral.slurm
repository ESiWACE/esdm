#!/bin/bash
#SBATCH -p compute2
#SBATCH --nodes=10
#SBATCH -A k20200
#SBATCH -d singleton
#SBATCH -m cyclic
#SBATCH -J esiwace

ulimit -s 102400


cd /home/dkrz/k202079/work/esdm
module load openmpi/1.8.4-gcc71 gcc/7.1.0 cmake betke/glib/2.48.0
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/deps/lib/pkgconfig


function clean(){
  echo $c
  srun  --ntasks-per-node=1 du -sh /tmp/_esdm
  srun  --ntasks-per-node=1 du -sh /home/dkrz/k202079/work/esdm/_esdm/
  srun  --ntasks-per-node=1 du -sh /dev/shm/_esdm

  srun --ntasks-per-node=1 rm -rf $PWD/_metadummy/
  srun --ntasks-per-node=1 rm -rf /tmp/_esdm
  srun --ntasks-per-node=1 rm -rf /home/dkrz/k202079/work/esdm/_esdm/
  srun --ntasks-per-node=1 rm -rf /dev/shm/_esdm
}

function run(){
  SIZE=$1
  PPN=$2
  r=$3
  srun --propagate=STACK --ntasks-per-node=$PPN ./readwrite-benchmark.exe $SIZE > output/$N-$PPN-$c-$SIZE-$r.txt
  clean
}

N=${SLURM_NNODES}

mkdir output
for r in $(seq 1 5) ; do
  for c in $(ls config) ; do
    ln -sf $PWD/config/$c _esdm.conf
    for s in 100 1000 10000 30000; do
      for ppn in 1 4 8 12 ; do
         run $s $ppn $r
      done
    done
  done
done
