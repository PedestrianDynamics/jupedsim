#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo add-apt-repository -y ppa:mhier/libboost-latest
    sudo apt-get update -qq
    sudo apt-get install libboost1.67-dev  -qq
elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
    brew install boost
elif [ "$TRAVIS_OS_NAME" == "windows" ]; then
    choco install boost-msvc-12
fi
