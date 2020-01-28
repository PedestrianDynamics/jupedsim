FROM ubuntu:18.04

RUN apt-get update && apt-get install -y wget \
                       git \
                       g++-8 \
                       cmake \
                       make \
                       libboost-dev \
                       python3 \
                       python3-pip \
                       python \
                       python-pip

RUN pip3 install numpy && \
    pip3 install matplotlib && \
    pip3 install pandas && \
    pip3 install dataclasses && \
    pip install numpy

# add user
RUN groupadd -r -g 1000 jupedsim && useradd -r -g jupedsim -u 1000 -m jupedsim
USER jupedsim

# clone code and build deps
RUN mkdir -p /home/jupedsim/workspace \
    cd /home/jupedsim/workspace \
    && git clone --depth=1 https://github.com/JuPedSim/jpscore.git \
    && cp jpscore/scripts/setup-deps.sh /opt \
    && cd /opt && CXX=/usr/bin/g++-8 CC=/usr/bin/gcc-8 ./setup-deps.sh

# build jpscore and jpsreport
RUN cd /home/jupedsim/workspace/jpscore \
    && mkdir -p build \
    && cd build \
    && cmake  ..\
    && make -j$(nproc)

