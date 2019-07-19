#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]
then
    if [ "$CXX" == "g++" ]
    then
        sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    elif [ "$CXX" == "clang++" ]
    then
        wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
        sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main"
    fi
    sudo apt-get update -qq;
elif [ "$TRAVIS_OS_NAME" == "osx" ]
then
    echo "not yet implemented."
fi
