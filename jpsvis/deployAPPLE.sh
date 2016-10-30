#!/bin/sh

RED='\033[0;31m'
NC='\033[0m' # No Color

if [ -d build ];then
    echo "INFO: found build directory"
    echo "INFO:${RED} rm build ${NC}"
    rm -rf build
    echo "INFO: mkdir build"
    mkdir build
fi

echo "INFO: cd build"
cd build

echo "INFO: actual directory: ${PWD}"

if [ -d ../bin ]; then
    echo "INFO: found bin directory"
    echo "INFO:${RED} remove ./bin directory ${NC}"
    rm -rf ../bin
fi

echo "INFO: cmake .."
cmake ..

echo "INFO: make .."
make -j4

echo "INFO: running  dynlibbundler .."
dylibbundler -od -b -x ../bin/JPSvis.app/Contents/MacOS/JPSvis -d ../bin/JPSvis.app/Contents/libs/

echo "INFO: running macdeployqt .."
macdeployqt ../bin/JPSvis.app -dmg
