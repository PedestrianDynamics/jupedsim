call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call C:\jpscore-deps\venv\Scripts\activate.bat
mkdir build
cd build
cmake .. -DPython_FIND_VIRTUALENV=ONLY -DCMAKE_TOOLCHAIN_FILE=C:/jpscore-deps/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=ON
cmake --build . --config Release --target unittests
cmake --build . --config Release --target package
