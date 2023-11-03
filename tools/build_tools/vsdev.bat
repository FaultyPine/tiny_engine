@echo off

set PreferredToolArchitecture=x64
set vcvarspath="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

:: if cl doesn't work, run vcvarsall
WHERE cl >nul 2>nul
IF NOT %ERRORLEVEL% == 0 (
    IF exist %vcvarspath% (
        call %vcvarspath% x64
    ) else (
        echo Couldn't find vcvarsall.bat! Change the path in tools/build_tools/vsdev.bat to the path to your visual studio installation.
    )
)