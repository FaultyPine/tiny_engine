@echo off

call "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" x64
cls
build.py %*