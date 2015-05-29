#!/bin/bash

COMPILE=0  # 1 -> compile first
if [ $COMPILE -eq 1 ];then
    make -f Makefile.cmake
fi

#./rebuild.exe  --inifile=ini.xml 
#./bin/jpscore --leak-check=full --inifile=inputfiles/Bottleneck/ini_bottleneck.xml
valgrind  ./bin/jpscore  --inifile=inputfiles/hybrid/hybrid_hall_ini.xml 
#./rebuild.exe  --inifile="inputfiles/arena/131021_arena_ini.xml" 

