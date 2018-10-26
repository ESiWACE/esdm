#/bin/bash


source env.bash

# Flags
# -r, --read                    Enable read benchmark
# -w, --write                   Enable write benchmark
# -u, --unlimited               Enable unlimited time dimension
# -F, --use-fill-value          Write a fill value
# --verify                      Verify that the data read is correct (reads the data again)
# 
# Optional arguments
# -n, --nn=0                    Number of nodes
# -p, --ppn=0                   Number of processes
# -d, --data-geometry=STRING    Data geometry `(t:x:y:z)`
# -b, --block-geometry=STRING   Block geometry `(t:x:y:z)`
# -c, --chunk-geometry=STRING   Chunk geometry `(t:x:y:z|auto)`
# -t, --io-type=ind             Independent / Collective I/O (ind|coll)
# -f, --testfile=STRING         Filename of the testfile
# -x, --output-format=human     Output-Format (parser|human)

mpiexec --np 4  benchtool -n=2 --ppn=2 -d=4:16:16:16 -f=outfile -r -w >> results.log
# 64 * (x:y:z)      *4 because assumes ints

#a=298; mpiexec --np 4  benchtool -n=2 --ppn=2 -d=1:$a:$a:$a -f=outfile -r -w >> results.log # ~100 MiB
#a=508; mpiexec --np 4  benchtool -n=2 --ppn=2 -d=1:$a:$a:$a -f=outfile -r -w >> results.log # ~500 MiB
#a=645; mpiexec --np 4  benchtool -n=2 --ppn=2 -d=1:$a:$a:$a -f=outfile -r -w >> results.log # ~1 GiB
#a=812; mpiexec --np 4  benchtool -n=2 --ppn=2 -d=1:$a:$a:$a -f=outfile -r -w >> results.log # ~2 GiB
#a=1389; mpiexec --np 4  benchtool -n=2 --ppn=2 -d=1:$a:$a:$a -f=outfile -r -w >> results.log # ~10 GiB

status=$?

# cleanup
rm -f outfile

exit status
