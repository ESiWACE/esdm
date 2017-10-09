# NetCDF Benchmark Tool

## Introduction
NetCDF Performance Benchmark Tool (NetCDF-Bench) was developed to measure NetCDF performance on devices ranging from notebooks to large HPC systems. It mimics the typical I/O behavior of scientific climate applications and captures the performance on each node/process. In the end, it aggregates the data to human readable summary. 

NetCDF-Bench supports independent I/O, collective I/O and chunked I/O modes. If necessary, it can pre-fill the variables with some value. 

## Domain decomposition
NetCDF-Bench supports various access patterns on a 4D dataset (one time dimension and three data dimensions). The short notation of the geometry is `(t:x:y:z)`, e.g., `(10:1000:1000:500)`.

The pictures shows an example of data with geometry `(3:6:4:3)`.
<center>
<img src="https://github.com/joobog/netcdf-bench/blob/master/doc/images/data.png" alt="Domain decomposition: 3:6:4:3" width="640">
</center>

### Data size vs. block size
The data is written in blocks to the shared file. The block size can be customized, but there are some restrictions. Assume, that
* `t:x:y:z` is the data size
* `t` is a multiple of some integer value `s`
* `nn` is the number of nodes
* `ppn` the number of processes per node
* `px` and `py` are integer values, that satisfies the condition: `px · py = nn · ppn`

Then `(s:x/px:y/py:z)` is a valid block size. (Default block size is `(1:x/nn:y/ppn:z)`.)

Each process allocates `(s:x/px:y/py:z) * type_size` memory space.

The data is read/written in timesteps.

## Usage
NetCDF-Bench is designed in that way that it can run without any parameters, but for advance usage our tool provides a number of parameters.

```
Benchtool (datatype: int) 
Synopsis: ./benchtool [-n] [-p] [-d] [-b] [-c] [-r] [-w] [-t] [-u] [-f] [-x] [-F] [--verify]  [Optional Args]

Flags
-r, --read                    Enable read benchmark
-w, --write                   Enable write benchmark
-u, --unlimited               Enable unlimited time dimension
-F, --use-fill-value          Write a fill value
--verify                      Verify that the data read is correct (reads the data again)

Optional arguments
-n, --nn=0                    Number of nodes
-p, --ppn=0                   Number of processes
-d, --data-geometry=STRING    Data geometry `(t:x:y:z)`
-b, --block-geometry=STRING   Block geometry `(t:x:y:z)`
-c, --chunk-geometry=STRING   Chunk geometry `(t:x:y:z|auto)`
-t, --io-type=ind             Independent / Collective I/O (ind|coll)
-f, --testfile=STRING         Filename of the testfile
-x, --output-format=human     Output-Format (parser|human)
```

## Example output
NetCDF-Bench aggregates the result of all processes and creates a summary. Here es an example output:

```
$ mpiexec -n 1 ./benchtool 
[1494497686.787267] [mlogin103:1936 :0]         sys.c:744  MXM  WARN  Conflicting CPU frequencies detected, using: 2501.00
Benchtool (datatype: int) 
Data geometry (t:x:y:z x sizeof(type))     100:100:100:10 x 4 bytes               
Block geometry (t:x:y:z x sizeof(type))      1:100:100:10 x 4 bytes               
Datasize                                             40000000 bytes                (40.0 MB)
Blocksize                                              400000 bytes                (400.0 kB)
I/O Access                                        independent
Storage                                            contiguous
File length                                             fixed
File value                                                 no
                                                                               min                  avg                  max                     
benchmark:write      Open time                                        0.2811507931         0.2811507931         0.2811507931 secs                
benchmark:write      I/O time                                         0.1901479111         0.1901479111         0.1901479111 secs                
benchmark:write      Close time                                       0.3576489800         0.3576489800         0.3576489800 secs                
benchmark:write      I/O Performance (w/o open/close)               200.6173638152       200.6173638152       200.6173638152 MiB/s               
benchmark:write      I/O Performance                                 46.0185526612        46.0185526612        46.0185526612 MiB/s 
```

## On-going work
We plan to extend the tool with the following features. 
1. Drop caches - Cached data can influence the results of I/O performance, therefore it must be cleared before benchmark runs. This features must run in user space.
2. Compression - This feature is useful to benchmakr the NetCDF compression, but also for people who are working on new compression methods.
3. CSV output - Detailed information about the benchmark run for analysis with third party tools.
