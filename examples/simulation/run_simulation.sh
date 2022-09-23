#!/bin/bash

EXPECTED_ARGS=2
jpsvis=../../build/bin/jpsvis.app/Contents/MacOS/jpsvis

if [ "$#" -eq "$EXPECTED_ARGS" ]
then
    echo "INFO : Running $0 with $1 $2"
    PYTHONPATH=../../build/lib:../../python_modules/jupedsim/ python3 $1
    if [ $? -eq 0 ]; then
        if [ $2 -eq 1 ]
        then
            echo "INFO : Run petrack2jpsvis converter"
            ../../scripts/petrack2jpsvis.py out.txt
            echo "INFO : run jpsvis"
            $jpsvis jps_out.txt
        fi
    else
        printf "\n -----\nSimulation errors!\n"
    fi
else
    printf "Invalid number of arguments.\nUsage %s python-script 0|1\n" $0
fi

     
