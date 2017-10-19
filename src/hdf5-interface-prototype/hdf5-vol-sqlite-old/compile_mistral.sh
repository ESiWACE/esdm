#!/bin/zsh

. /sw/rhel6-x64/tcl/modules-3.2.10/Modules/3.2.10/init/zsh 

setopt SH_WORD_SPLIT

module purge
module load intel/17.0.1
module load fca/2.5.2431
module load mxm/3.4.3082
module load bullxmpi_mlx_mt/bullxmpi_mlx_mt-1.2.9.2
module load betke/hdf5-vol
module load betke/sqlite3
module list


SQLITEDIR="$(readlink -f $(dirname $(which sqlite3))/..)"
MPIPWD="$(readlink -f $(dirname $(which mpiexec))/..)"
ROOTDIR="${PWD}/.."

CC="$(which mpicc)"

LDFLAGS=" \
	-L${ROOTDIR}/hdf5-vol-install/lib \
	-lhdf5 \
	-Wl,-rpath=$SQLITEDIR/lib \
	$(pkg-config --libs sqlite3)"

	#-g3 -DDEBUG -DTRACE \
CFLAGS=" \
	-std=gnu11 \
	-D_LARGEFILE64_SOURCE \
	-I${ROOTDIR}/hdf5-vol-install/include \
	-I$MPIPWD/include \
	-I${ROOTDIR}/ext_plugin \
	$(pkg-config --cflags sqlite3)"



[ ! -d shared ] && mkdir -p shared
[ ! -d multifile ] && mkdir -p multifile

set -x
#$CC $CFLAGS $LDFLAGS -shared -fpic -o shared/libh5-sqlite-plugin.so  h5_sqlite_plugin.c db_sqlite.c base.c
#$CC $CFLAGS $LDFLAGS -shared -fpic -DMULTIFILE -o multifile/libh5-sqlite-plugin.so  h5_sqlite_plugin.c db_sqlite.c base.c
$CC $CFLAGS $LDFLAGS -shared -fpic -DADAPTIVE -o adaptive/libh5-sqlite-plugin.so  h5_sqlite_plugin.c db_sqlite.c esdm.c base.c
set +x
