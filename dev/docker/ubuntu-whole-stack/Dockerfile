FROM ubuntu:20.04

# Requirements and dependencies easily obtained with the distribution
RUN apt-get update && apt-get install -y build-essential git cmake libglib2.0 libglib2.0-dev libjansson4 libjansson-dev openmpi-bin libopenmpi-dev libtool libtool-bin

ADD data /data
# Requirements and ESDM compilation from source
WORKDIR "/data"

# Install ESDM
RUN git clone --recurse-submodules https://github.com/ESiWACE/esdm.git
RUN cd esdm && ./configure --debug --prefix=/data/install && cd build && make -j 4 && make -j 4 install

# Install ESDM-NetCDF
RUN apt-get install -y libhdf5-openmpi-dev hdf5-tools wget python3-setuptools
RUN git clone --recurse-submodules "https://github.com/ESiWACE/esdm-netcdf-c"
WORKDIR "/data/esdm-netcdf-c"
RUN ./bootstrap 
RUN ./configure --prefix=/data/install --enable-hdf5 --enable-esdm --with-esdm=/data/install CFLAGS="-I/usr/include/hdf5/openmpi/ -g3" LDFLAGS="-L/usr/lib/x86_64-linux-gnu/hdf5/openmpi/" CC=mpicc  --disable-dap
RUN make -j
RUN make install

# Install ESDM-NetCDF-Python
ENV PKG_CONFIG_PATH=/data/install/lib/pkgconfig:$PKG_CONFIG_PATH
ENV C_INCLUDE_PATH=/usr/lib/x86_64-linux-gnu/openmpi/include/:/data/install/include/:$C_INCLUDE_PATH
ENV LD_LIBRARY_PATH=/data/install/lib/:$LD_LIBRARY_PATH
ENV PATH=/data/install/bin:$PATH
RUN apt install -y python3-numpy python3-pip cython3 python3-mpi4py
WORKDIR "/data/esdm-netcdf-c/dev/"
RUN git clone https://github.com/Unidata/netcdf4-python.git
WORKDIR "./netcdf4-python"
# The patch was tested with this netcdf4-python version
# Newer versions might be working as well
RUN git checkout 3bce557cbdbf77805b088fe76ccd7d312d643159
RUN patch -l -s -p1 < ../v2.patch
RUN python3 setup.py install --user

# Set some environment variables
ENV ESDM_SRC_DIR /data

# Default action when running the container
WORKDIR "/data"
CMD cd esdm/build && make test
