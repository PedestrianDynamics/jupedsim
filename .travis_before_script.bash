#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo add-apt-repository -y ppa:apokluda/boost1.53
    sudo apt-get update
    sudo apt-get install libboost1.53-all-dev
    sudo apt-get install doxygen
    sudo apt-get install lcov
    sudo pip install codecov
    sudo pip install numpy
elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    brew update;   
    brew unlink boost
    brew install boost
    brew install doxygen
    brew install lcov
    sudo pip install numpy
fi
