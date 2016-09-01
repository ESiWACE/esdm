#!/bin/sh

# Time limit is one minute.
# See "man sbatch" for other time formats.
#SBATCH --time=180
# Run 1 tasks on 1 nodes.
#SBATCH -N 1
# Output goes to job.out, error messages to job.err.
#SBATCH --error=lustre_rand_read_dummy_1m.err --output=lustre_rand_read_dummy_1m.out

srun hostname
rm -f $1
./workload-random-dummy $1 $2 $3 $4

