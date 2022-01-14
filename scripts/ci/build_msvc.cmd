mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=C:/vcpkg/scripts/installed/x64-windows-release -DBUILD_TESTS=ON
cmake --build . --config Release -j
cmake --build . --config Release -t unittests 
cmake --build . --config Release -t package
