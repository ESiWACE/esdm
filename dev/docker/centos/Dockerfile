FROM centos:centos7

# Requirements and dependencies easily obtained with the distribution
# gcc gcc-c++ cmake
RUN yum -y install make automake centos-release-scl devtoolset-8 git glib2 glib2-devel jansson jansson-devel mpich-devel wget

# Requirements and ESDM compilation from source
WORKDIR "/data"
RUN git clone --recurse-submodules https://github.com/ESiWACE/esdm.git

# Install CMAKE (> 2.8)
RUN wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz && tar -zxf cmake-3.* && cd cmake-3.* && ./bootstrap --prefix=/usr/local && make -j4 && make install

# Install ESDM

RUN cd esdm && PATH=/opt/rh/devtoolset-8/root/usr/bin/:$PATH:/usr/lib64/mpich/bin ./configure --debug --prefix=/data/install && cd build && make -j 4 && make -j 4 install

# ADD data /data

# Set some environment variables
ENV ESDM_SRC_DIR /data

# Default action when running the container
WORKDIR "/data"
CMD cd esdm/build && make test
