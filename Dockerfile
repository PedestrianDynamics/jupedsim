# -----------------------------------------
# - build:
#   docker build jupedsim/jpscore .
# - run:
#    docker -it jupedsim/jpscore
# -----------------------------------------
    
FROM ubuntu:14.04
 
MAINTAINER Mohcine Chraibi <m.chraibi@gmail.com>

# install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    git \
    lcov \
    doxygen \
    libboost-system-dev \
    libboost-filesystem-dev \
    libboost-test-dev \
    python \
    python-dev \
    python-pip \
    && apt-get clean

# set environment
ENV BOOST_ROOT /usr
ENV HOME /home/jupedsim

# add user
RUN groupadd -r -g 1000 jupedsim && useradd -r -g jupedsim -u 1000 -m jupedsim
USER jupedsim

# install jpscore
RUN mkdir -p /home/jupedsim/workspace
RUN cd /home/jupedsim/workspace \
    && git clone --depth=5 https://cst.version.fz-juelich.de/jupedsim/jpscore.git \
    && cd jpscore \
    && mkdir -p build \
    && cd build \
    && cmake -DBUILD_TESTING=ON ..\
    && make


# install python variant filtering dependencies
#RUN pip install numpy
#RUN pip install matplotlib


# by default /bin/bash is executed
CMD /bin/bash && echo "JuPedSim/jpscore: Enjoy simulating!"
