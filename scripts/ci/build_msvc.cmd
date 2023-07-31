call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call C:\Python\Python311\python.exe -m venv C:\jupedsim-venv311
call C:\jupedsim-venv311\Scripts\activate.bat
mkdir build
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build . --config Release
cmake --build . --config Release --target unittests
