FROM centos:centos7

# Requirements and dependencies easily obtained with the distribution
RUN yum -y install make automake centos-release-scl devtoolset-8 git glib2 glib2-devel jansson jansson-devel mpich-devel wget zlib-devel gcc-c++ gcc

# Requirements and ESDM compilation from source
WORKDIR "/data"
RUN git clone --recurse-submodules https://github.com/ESiWACE/esdm.git

# Install CMAKE (> 2.8)
RUN wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz && tar -zxf cmake-3.* && cd cmake-3.* && export PATH=/opt/rh/devtoolset-8/root/usr/bin/:$PATH:/usr/lib64/mpich/bin && ./bootstrap --prefix=/usr/local && make -j4 && make install

# Install HDF5
RUN wget https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.10.5.tar.gz && tar -xzf hdf5*.gz
RUN cd hdf5-1.10.5 && export PATH=/opt/rh/devtoolset-8/root/usr/bin/:$PATH:/usr/lib64/mpich/bin && ./configure --enable-parallel --prefix=/usr/local && make -j 4 && make -j 4 install


# Install GCC8
RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-8
RUN scl enable devtoolset-8 -- bash

# Install ESDM
RUN cd esdm && export PATH=/opt/rh/devtoolset-8/root/usr/bin/:$PATH:/usr/lib64/mpich/bin && gcc -v && ./configure --debug --prefix=/data/install && cd build && make -j 4 && make -j 4 install

# Install ESDM-NetCDF
ENV PATH=/opt/rh/devtoolset-8/root/usr/bin/:$PATH:/usr/lib64/mpich/bin
RUN yum -y install libtool
WORKDIR "/data"
RUN git clone --recurse-submodules https://github.com/ESiWACE/esdm-netcdf-c.git
WORKDIR "/data/esdm-netcdf-c"
RUN ./bootstrap
RUN mkdir build
WORKDIR ./build
RUN ../configure --prefix=/data/install --with-esdm=/data/install CFLAGS="-I/usr/include/hdf5/openmpi/ -g3" LDFLAGS="-L/usr/lib/x86_64-linux-gnu/hdf5/openmpi/" CC=mpicc  --disable-dap 
RUN make -j install


# Install ESDM-NetCDF-Python
ENV PKG_CONFIG_PATH=/data/install/lib/pkgconfig:$PKG_CONFIG_PATH
ENV C_INCLUDE_PATH=/usr/include/mpich-x86_64/:/data/install/include/:$C_INCLUDE_PATH
ENV LD_LIBRARY_PATH=/data/install/lib/:$LD_LIBRARY_PATH
ENV PATH=/data/install/bin:$PATH
RUN yum -y install patch python3-devel
RUN pip3 install numpy
RUN pip3 install mpi4py
WORKDIR "/data/esdm-netcdf-c/dev/"
RUN git clone https://github.com/Unidata/netcdf4-python.git
WORKDIR "/data/esdm-netcdf-c/dev/netcdf4-python"
# The patch was tested with this netcdf4-python version
# Newer versions might be working as well
RUN git checkout 3bce557cbdbf77805b088fe76ccd7d312d643159
RUN patch -l -s -p1 < ../v2.patch
RUN python3 setup.py install --user

ADD data /data

# Set some environment variables
ENV ESDM_SRC_DIR /data

# Default action when running the container
WORKDIR "/data"
CMD cd esdm/build && make test
