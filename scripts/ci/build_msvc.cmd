call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
rustup default stable
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/jpscore-deps/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=ON
cmake --build . --config Release --target unittests 
cmake --build . --config Release --target package
