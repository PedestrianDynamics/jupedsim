FROM ubuntu:21.10
RUN apt-get update && \
    apt-get install -y software-properties-common

RUN apt-get update && \
    apt-get install -y \
    wget \
    git \
    g++ \
    cmake \
    make \
    ninja-build \
    clang \
    clang-format \
    libboost-dev \
    libcgal-dev \
    python3 \
    python3-pip \
    python3-matplotlib \
    python3-numpy \
    python3-pandas \
    python3-scipy

COPY scripts/setup-deps.sh /opt/

RUN cd /opt && ./setup-deps.sh

RUN pip3 install dataclasses
