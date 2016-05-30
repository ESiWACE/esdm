#!/bin/sh

diff -cB ../install/download/netcdf-4.4.0/libsrc4/nc4file.c.orig ../install/download/netcdf-4.4.0/libsrc4/nc4file.c > netcdf4-libsrc4-nc4file-c.patch
diff -cB ../install/download/netcdf-4.4.0/include/netcdf.h.orig ../install/download/netcdf-4.4.0/include/netcdf.h > netcdf4-include-netcdf-h.patch
