FROM ubuntu:20.04

# Requirements and dependencies easily obtained with the distribution
RUN apt-get update && apt-get install -y build-essential git cmake libglib2.0 libglib2.0-dev libjansson4 libjansson-dev openmpi-bin libopenmpi-dev libtool libtool-bin

ADD data /data
# Requirements and ESDM compilation from source
WORKDIR "/data"
#
RUN git clone --recurse-submodules https://github.com/ESiWACE/esdm.git
RUN cd esdm && ./configure --debug --prefix=/data/install && cd build && make -j 4 && make -j 4 install

RUN apt-get install -y libhdf5-openmpi-dev hdf5-tools wget
RUN git clone --recurse-submodules https://github.com/ESiWACE/esdm-netcdf-c.git
RUN cd esdm-netcdf && autoreconf && cd build && ../configure --prefix=/data/install --with-esdm=/data/install CFLAGS="-I/usr/include/hdf5/openmpi/ -g3" LDFLAGS="-L/usr/lib/x86_64-linux-gnu/hdf5/openmpi/" CC=mpicc  --disable-dap
RUN cd esdm-netcdf/build && make -j 2 install

# Set some environment variables
ENV ESDM_SRC_DIR /data

# Default action when running the container
WORKDIR "/data"
CMD cd esdm/build && make test
