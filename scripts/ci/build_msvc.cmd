mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/admin/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=ON
cmake --build . --config Release
cmake --build . --config Release -t package
cmake --build . --config Release -t tests 
