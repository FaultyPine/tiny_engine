@echo off

set PreferredToolArchitecture=x64
:: if cl doesn't work, run vcvarsall
WHERE cl >nul 2>nul
IF NOT %ERRORLEVEL% == 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)