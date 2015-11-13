echo ">> cd build"
cd build

pwd

echo ">> cmake .."
cmake ..

echo "make .."
make -j4

echo "dynlibbundler .."
dylibbundler -od -b -x ../bin/JPSvis.app/Contents/MacOS/JPSvis -d ../bin/JPSvis.app/Contents/libs/

echo "macdeployqt .."
macdeployqt ../bin/JPSvis.app -dmg
