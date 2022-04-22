FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y software-properties-common

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    wget \
    unzip \
    git \
    g++ \
    cmake \
    make \
    ninja-build \
    clang \
    clang-format-14 \
    libglm-dev \
    qtbase5-dev \
    libvtk9-dev \
    libvtk9-qt-dev \
    libopengl-dev \
    python3 \
    python3-pip \
    python3-matplotlib \
    python3-numpy \
    python3-pandas \
    python3-scipy \
    python3-pytest

RUN cd /usr/bin && ln -s pytest-3 pytest

COPY scripts/setup-deps.sh requirements.txt /opt/

RUN cd /opt && ./setup-deps.sh

RUN pip3 install -r /opt/requirements.txt
