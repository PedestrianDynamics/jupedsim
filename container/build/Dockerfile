FROM ubuntu:18.04
RUN apt-get update && apt-get install -y software-properties-common

RUN apt-get update && apt-get install -y wget \
                       git \
                       g++-8 \
                       clang-8 \
                       cmake \
                       make \
                       libboost-dev \
                       libcgal-dev \
                       clang-format-8 \
                       python3 \
                       python3-pip \
                       python \
                       python-pip

COPY scripts/setup-deps.sh /opt/

RUN cd /opt && CXX=/usr/bin/g++-8 CC=/usr/bin/gcc-8 ./setup-deps.sh

RUN pip3 install numpy && \
    pip3 install matplotlib && \
    pip3 install pandas && \
    pip install numpy