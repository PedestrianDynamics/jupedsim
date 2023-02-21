call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call C:\jpscore-deps\venv\Scripts\activate.bat
set PATH=%PATH%;C:\jpscore-deps\x64-windows\bin
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=C:/jpscore-deps/x64-windows -DBUILD_TESTS=ON -DBUILD_JPSVIS=OFF
cmake --build . --config Release --target unittests
