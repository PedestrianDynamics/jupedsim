FROM ubuntu:21.10

RUN apt-get update && \
    apt-get install -y software-properties-common

RUN apt-get update && \
    apt-get install -y \
    wget \
    git \
    g++ \
    mingw-w64 \
    cmake \
    make \
    ninja-build \
    clang \
    clang-format \
    python3 \
    python3-pip \
    python3-matplotlib \
    python3-numpy \
    python3-pandas \
    python3-scipy \
    python3-pytest

RUN cd /usr/bin && ln -s pytest-3 pytest

COPY scripts/setup-deps.sh /opt/

RUN cd /opt && ./setup-deps.sh && ./setup-deps.sh --install-path /opt/windows-deps --compile-for-windows

RUN pip3 install dataclasses
