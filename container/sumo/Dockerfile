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
    python3 \
    python3-pip \
    python3-matplotlib \
    python3-numpy \
    python3-pandas \
    python3-scipy \
    python3-pytest \
    libxerces-c-dev \
    libfox-1.6-dev \
    libgdal-dev \
    libproj-dev \
    libgl2ps-dev \
    swig

RUN cd /usr/bin && ln -s pytest-3 pytest

COPY scripts/setup-deps.sh requirements.txt /opt/

RUN cd /opt && ./setup-deps.sh --install-path /opt/jpscore-deps

RUN pip3 install -r /opt/requirements.txt

RUN git config --global --add safe.directory '*'
