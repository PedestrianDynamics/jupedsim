# -----------------------------------------
# - build:
#   docker build jupedsim/jpsvis .
# - run:
#    docker -it jupedsim/jpsvis
# -----------------------------------------
    
FROM ubuntu:14.04
 
MAINTAINER Mohcine Chraibi <m.chraibi@gmail.com>

# install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    git \
    doxygen \
    qt5-default \
    qttools5-dev-tools \
    libvtk5-dev \
    libvtk5-qt4-dev \
    && apt-get clean

# set environment
ENV HOME /home/jupedsim

# add user
RUN groupadd -r -g 1000 jupedsim && useradd -r -g jupedsim -u 1000 -m jupedsim
USER jupedsim
# sudo usermod -p `perl -e "print crypt("password","Q4")"` root

# install jpscore
RUN mkdir -p /home/jupedsim/workspace
RUN cd /home/jupedsim/workspace \
    && git clone --depth=5 https://cst.version.fz-juelich.de/jupedsim/jpsvis.git \
    && cd jpsvis \
    && mkdir -p build \
    && cd build \
    && cmake  ..\
    && make



# by default /bin/bash is executed
CMD /bin/bash && echo "Thank you for using JuPedSim/jpsvis!"
