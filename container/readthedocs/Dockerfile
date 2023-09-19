FROM ubuntu:23.04

# Install dependencies
RUN apt-get update && \
    apt-get install -y software-properties-common
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    wget \
    unzip \
    git \
    g++ \
    cmake \
    ninja-build \
    python3-full \
    python3-pip \
    linux-tools-common \
    linux-tools-generic 

