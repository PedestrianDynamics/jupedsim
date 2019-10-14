FROM ubuntu:18.04
RUN apt-get update

RUN apt-get install -y software-properties-common

RUN apt-get install -y wget \
                       git \
                       g++-8 \
                       clang-8 \
                       cmake \
                       make \
                       libboost-dev \
                       libcgal-dev

COPY scripts/setup-deps.sh /opt/

RUN cd /opt && CXX=/usr/bin/g++-8 CC=/usr/bin/gcc-8 ./setup-deps.sh