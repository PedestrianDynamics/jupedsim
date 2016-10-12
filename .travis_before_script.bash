#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo apt-get update
    sudo apt-get install doxygen
    sudo apt-get install lcov
    sudo apt get install wget
    sudo apt-get install libbz2-dev
    sudo pip install codecov
    sudo pip install numpy
    wget http://downloads.sourceforge.net/project/boost/boost/1.61.0/boost_1_61_0.tar.gz \
        && tar xfz boost_1_61_0.tar.gz \
        && rm boost_1_61_0.tar.gz \
        && cd boost_1_61_0 \
        && ./bootstrap.sh --with-libraries=filesystem,test,system\
        && sudo ./b2 --without-python --prefix=/usr -j 4 link=shared runtime-link=shared install \
        && cd .. && sudo rm -rf boost_1_61_0 && sudo ldconfig

elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    brew update;   
    brew unlink boost
    brew install boost
    brew install doxygen
    brew install lcov
    sudo pip install numpy
fi
