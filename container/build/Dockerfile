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

RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 6084F3CF814B57C1CF12EFD515CF4D18AF4F7421 && \
    add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main" && \
    apt-get update && \
    apt-get install -y clang-format-9


COPY scripts/setup-deps.sh /opt/

RUN cd /opt && CXX=/usr/bin/g++-8 CC=/usr/bin/gcc-8 ./setup-deps.sh

RUN pip3 install numpy && \
    pip3 install matplotlib && \
    pip3 install pandas && \
    pip3 install dataclasses && \
    pip3 install scipy && \
    pip install numpy && \
    pip install scipy