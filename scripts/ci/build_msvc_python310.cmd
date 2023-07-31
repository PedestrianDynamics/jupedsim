call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
rm -rf C:\jupedsim-venv310
call C:\Python\Python310\python.exe -m venv C:\jupedsim-venv310
call C:\jupedsim-venv310\Scripts\activate.bat
mkdir build
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build . --config Release
cmake --build . --config Release --target unittests
