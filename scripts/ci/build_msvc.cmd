call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call C:\jpscore-deps\venv\Scripts\activate.bat
set PATH=%PATH%;C:\jpscore-deps\x64-windows\bin
mkdir build
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build . --config Release
cmake --build . --config Release --target unittests
