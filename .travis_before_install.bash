#!/usr/bin/env bash

# install g++4.8.1 and clang 3.4

if [ "$TRAVIS_OS_NAME" == "linux" ]
then
    if [ "$CXX" == "g++" ]
    then
        sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    elif [ "$CXX" == "clang++" ]
    then
        sudo add-apt-repository -y ppa:h-rayflood/llvm
    fi
    sudo apt-get update -qq;
elif [ "$TRAVIS_OS_NAME" == "osx" ]
then
    echo "not yet implemented."
fi
