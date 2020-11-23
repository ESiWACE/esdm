FROM ubuntu:20.04

ADD data /data

# Requirements and dependencies easily obtained with the distribution
RUN apt-get update && apt-get install -y build-essential git cmake libglib2.0 libglib2.0-dev libjansson4 libjansson-dev openmpi-bin libopenmpi-dev libtool libtool-bin

# Requirements and ESDM compilation from source
WORKDIR "/data"
#
RUN git  clone --recurse-submodules https://github.com/ESiWACE/esdm.git
RUN cd esdm && ./configure && cd build && make

# Set some environment variables
ENV ESDM_SRC_DIR /data


# Default action when running the container
WORKDIR "/data/esdm"
CMD cd build && make test
