FROM fedora

ADD data /data

# Requirements and dependencies easily obtained with the distribution
RUN dnf -y install make automake gcc gcc-c++ kernel-devel  cmake git && dnf -y install glib2 glib2-devel mpi jansson jansson-devel

# Requirements and ESDM compilation from source
WORKDIR "/data"
RUN git clone https://github.com/ESiWACE/esdm.git && cd esdm && ./configure && cd build && make && make test


# Set some environment variables
ENV ESDM_SRC_DIR /data


# Default action when running the container
WORKDIR "/data/esdm"
CMD cd build && make test
