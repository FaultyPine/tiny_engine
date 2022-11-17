@echo off

cls

:: if cl doesn't work, run vcvarsall
WHERE cl >nul 2>nul
IF NOT %ERRORLEVEL% == 0 (
    call "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" x64
)

build.py %*