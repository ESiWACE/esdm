#!/bin/zsh
#
setopt SH_WORD_SPLIT

SCRIPT="$(readlink -f $0)"
SCRIPTPATH="$(dirname $SCRIPT)"
cd $SCRIPTPATH

module purge
module load intel/16.0.3 
module load fca/2.5.2393
module load mxm/3.3.3002
module load bullxmpi_mlx_mt/bullxmpi_mlx_mt-1.2.8.3
module load betke/hdf5/1.8.17
module load betke/netcdf/4.4.0

export TESTFILE="../test/testfile.nc"
#export TESTFILE="/dev/shm/test/testfile.nc"
export TESTDIR="$(dirname $TESTFILE)"
export BENCHTOOL="../install/bin/benchtool"

if [ -z ${SLURM_NNODES+x} ]
then
	echo "USE MPIEXEC"
	export NN=2
	export PPN=2
else
	echo "USE SRUN"
	export NN=$SLURM_NNODES
	export PPN=$SLURM_NTASKS_PER_NODE
fi

export X="$(($NN*240/$NN))"
export Y="$(($PPN*240/$PPN))"
export Z=20
export T=10

[ -d $TESTDIR ] && rm -rf $TESTDIR
mkdir -p $TESTDIR &> /dev/null
lfs setstripe -c $(($NN * 6)) $TESTDIR
lfs getstripe $TESTDIR


export PATH=/opt/mpi/bullxmpi_mlx/1.2.8.3/bin/:$PATH
MPIEXEC=$(which mpiexec)
SRUN=$(which srun)

if [ -z ${SLURM_NNODES+x} ]
then 
	#echo "USE MPIEXEC"
	set -x
	#${MPIEXEC} -np $(($NN * $PPN)) ${BENCHTOOL} -n=$NN -p=$PPN -d="$T:$X:$Y:$Z" -b="1:1:1:1" -c="1:$(($X/$NN)):$(($Y/$PPN)):$Z" -w -r --testfile=$TESTFILE
	${MPIEXEC} -np $(($NN * $PPN)) ${BENCHTOOL} -n=$NN -p=$PPN -d="$T:$X:$Y:$Z" -c=auto -t=coll -u -w -r --testfile=$TESTFILE
	set +x
else 
	#echo "USE SRUN"
	set -x
	${SRUN} --nodes=$NN --ntasks-per-node=$PPN ${BENCHTOOL} -n=$NN -p=$PPN -d="$T:$X:$Y:$Z" -c=auto -w -r --testfile=$TESTFILE
	set +x
fi

