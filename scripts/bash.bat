@echo off
%HOMEDRIVE%
cd %HOMEPATH%
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
"C:\Program Files\Git\bin\bash.exe" --login -i