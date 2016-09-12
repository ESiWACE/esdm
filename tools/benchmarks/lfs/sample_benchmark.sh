#!/bin/bash

echo "batching slurm files!!!"

sbatch --nodelist=west[1] rand_lfs.slurm /tmp/datafile7.df 8192 8192000 10

echo "batching slurm files finished!"

echo "here is the queue for your jobs"
squeue


