#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]
then
    sudo apt install -y \
         g++-9 \
         libboost-dev \
         libboost-test-dev \
         libcgal-dev
    if [ "$TOOLCHAIN" = "gcc" ];
    then
        export CXX="g++-9"
    elif [ "$TOOLCHAIN" == "clang" ]
    then
        sudo apt install -y clang-8 libomp-8-dev
        export CXX="clang++-8";
    fi
elif [ "$TRAVIS_OS_NAME" == "osx" ]
then
    echo "not yet implemented"
fi
