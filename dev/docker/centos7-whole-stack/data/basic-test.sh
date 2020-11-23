#!/bin/bash
wget https://www.unidata.ucar.edu/software/netcdf/examples/tos_O1_2001-2002.nc
export PATH=$PATH:/data/install/bin

echo '{"esdm":
{
"backends": [
{"type": "POSIX", "id": "tmp", "target": "./_esdm",
"performance-model" : {"latency" : 0.000001, "throughput" : 500.0},
"max-threads-per-node" : 4,
"max-fragment-size" : 104857600,
"max-global-threads" : 8,
"accessibility" : "global"
}
],
"metadata": {"type": "metadummy",
"id": "md",
"target": "./_metadummy",
  "accessibility" : "global"}
}
}' > esdm.conf

mkfs.esdm -g -l --create  --remove --ignore-errors
nccopy tos_O1_2001-2002.nc esdm://tos
ncdump -h esdm://tos
