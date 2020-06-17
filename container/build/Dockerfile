FROM ubuntu:20.04
RUN apt-get update && apt-get install -y software-properties-common

RUN apt-get update && apt-get install -y wget \
                       git \
                       g++ \
                       clang-8 \
                       cmake \
                       make \
                       libboost-dev \
                       libcgal-dev \
                       python3 \
                       python3-pip \
                       python3-matplotlib

RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 6084F3CF814B57C1CF12EFD515CF4D18AF4F7421 && \
    add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-10 main" && \
    apt-get update && \
    apt-get install -y clang-format-10


COPY scripts/setup-deps.sh /opt/

RUN cd /opt && CXX=/usr/bin/g++-9 CC=/usr/bin/gcc-9 ./setup-deps.sh

RUN pip3 install numpy && \
    pip3 install pandas && \
    pip3 install dataclasses && \
    pip3 install scipy