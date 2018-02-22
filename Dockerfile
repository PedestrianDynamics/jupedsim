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
    wget \
    libbz2-dev \ 
    python \
    python-dev \
    python-pip \
    libcgal-dev \
    && apt-get clean

# set environment
ENV HOME /home/jupedsim

ARG boost_version=1.61.0
ARG boost_dir=boost_1_61_0
ENV boost_version ${boost_version}
ENV BOOST_ROOT /usr
RUN wget http://downloads.sourceforge.net/project/boost/boost/${boost_version}/${boost_dir}.tar.gz \
    && tar xfz ${boost_dir}.tar.gz \
    && rm ${boost_dir}.tar.gz \
    && cd ${boost_dir} \
    && ./bootstrap.sh --with-libraries=filesystem,test,system\
    && ./b2 --without-python --prefix=/usr -j 4 link=shared runtime-link=shared install \
    && cd .. && rm -rf ${boost_dir} && ldconfig

# add user
RUN groupadd -r -g 1000 jupedsim && useradd -r -g jupedsim -u 1000 -m jupedsim
USER jupedsim
# sudo usermod -p `perl -e "print crypt("password","Q4")"` root

# install jpscore
RUN mkdir -p /home/jupedsim/workspace
RUN cd /home/jupedsim/workspace \
    && git clone --depth=5 https://gitlab.version.fz-juelich.de/jupedsim/jpscore.git \
    && cd jpscore \
    && mkdir -p build \
    && cd build \
    && cmake -DBUILD_TESTING=ON ..\
    && make


# install python variant filtering dependencies
#RUN pip install numpy
#RUN pip install matplotlib


# by default /bin/bash is executed
CMD /bin/bash && echo "Thank you for using JuPedSim/jpscore!"
