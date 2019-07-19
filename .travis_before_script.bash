#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo add-apt-repository -y ppa:mhier/libboost-latest
    sudo apt-get update -qq
    sudo apt-get install libboost1.67-dev  -qq
    sudo apt-get install doxygen -qq
    wget http://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
    chmod +x miniconda.sh
    ./miniconda.sh -b
    export PATH=/home/travis/miniconda3/bin:$PATH
    conda install --yes numpy matplotlib pandas
elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    brew install boost
    brew install doxygen
    pip install numpy
    pip install matplotlib
    pip install pandas
elif [ "$TRAVIS_OS_NAME" == "windows" ]; then
    choco install boost-msvc-12
    choco install doxygen.install
    choco install numpy
    choco install matplotlib
    choco install pandas
fi
