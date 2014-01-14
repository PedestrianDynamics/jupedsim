#!/bin/bash

COMPILE=0  # 1 -> compile first
if [ $COMPILE -eq 1 ];then
    make
fi

#./rebuild.exe  --inifile=ini.xml 
#./bin/jpscore  --inifile=inputfiles/Bottleneck/ini_bottleneck.xml
./bin/jpscore  --inifile=demos/scenario_1/ini.xml 
#./rebuild.exe  --inifile="inputfiles/arena/131021_arena_ini.xml" 
