
RED='\033[0;31m'
NC='\033[0m' # No Color

CMD="jpsvis"


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


echo "INFO: running  <dylibbundler -od -b -x ../bin/${CMD}.app/Contents/MacOS/${CMD} -d ../bin/${CMD}.app/Contents/libs/>"
dylibbundler -od -b -x ../bin/${CMD}.app/Contents/MacOS/${CMD} -d ../bin/${CMD}.app/Contents/libs/


# dylibbundler has a problem linking to local python framework. So this quick fix..
isPythonDependencyGlobal=`otool -L ../bin/${CMD}.app/Contents/MacOS/${CMD} | grep Python.framework`

echo "isPythonDependencyGlobal: <$isPythonDependencyGlobal>"

if [[ -n $isPythonDependencyGlobal ]];then
    if [[ ! $isPythonSWDependencyGlobal == *"@executable_path"* ]]; then
        #install_name_tool -change /usr/local/opt/python3/Frameworks/Python.framework/Versions/3.5/Python
        #                           @executable_path/../Frameworks/Python.framework/Versions/3.5/Python
        #                           ./bin/jpsvis.app/Contents/MacOS/jpsvis
        
        Python=`echo ${isPythonDependencyGlobal} | xargs | awk '{print $1}'`
        echo "WARNING: ${RED} <$Python> is not relative to ${CMD} ${NC}."
        
        Frameworks=`echo ${Python} |  awk -F Frameworks '{ print "Frameworks"$2 }'`
        echo "WARNING: ${RED} change <$Python> to <@executable_path/../$Frameworks> ${CMD} ${NC}."

        install_name_tool -change $Python @executable_path/../$Frameworks ../bin/${CMD}.app/Contents/MacOS/${CMD}

        echo "INFO: Check again"
        otool -L ../bin/${CMD}.app/Contents/MacOS/${CMD} | grep Python.framework
        echo "-----------------"

    fi
fi

# check if dependencies to libs are local to .app
python checkDependencies.py 

appdmg Resources/dmg.json {CMD}-0.8.1.dmg
