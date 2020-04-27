# requirements:
# - dylibbundler (for vtk)
# - macdeployqt (for qt)
# - appdmg
#-----------------
RED='\033[0;31m'
NC='\033[0m' # No Color
CMD="jpsvis"
VERSION="0.8"
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
cmake -DVTK_DIR=/Users/chraibi/workspace/VTK-8.2.0/build ..

echo "INFO: make .."
make -j4

# for vtk libs
dylibbundler -od -b -x ../bin/${CMD}.app/Contents/MacOS/${CMD} -d ../bin/${CMD}.app/Contents/libs/

# deploy qt frameworks. dylibbundler can't do this.
macdeployqt  ../bin/${CMD}.app/ -verbose=2

# make dmg
appdmg  ../Resources/dmg.json   ${CMD}-${VERSION}.dmg
