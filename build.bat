@echo off


set APP_NAME=app

set SOURCES=src/main.cpp
set LINKER_ARGS= -Llib/glfw -lglfw3 -lpthread -lgdi32
:: flags are shared for both our main compilation and the pch's compilation
set COMPILER_ARGS= -g -Iinclude -std=c++11

set BUILD_COMMAND=g++ -o build/%APP_NAME%.exe %COMPILER_ARGS% %SOURCES% %LINKER_ARGS%

echo Building...

:: building precompiled header
if "%1" == "pch" (
    set PCH_FILE=src/tiny_engine/pch.h
    g++ -x c++-header -o %PCH_FILE%.gch -c %PCH_FILE% %COMPILER_ARGS%
    echo Rebuilt precompiled header! %PCH_FILE%
    Goto :eof
)

:: output the amount of time it took to run the build command
if "%1" == "timed" (
    powershell -Command "(Measure-Command {cmd.exe /c %BUILD_COMMAND% | Out-Default}).ToString()"
    Goto :eof
)

if "%1" == "run" (
    Goto :runapp
)

%BUILD_COMMAND% || Goto :eof


echo Built!


if "%1" == "norun" (
    Goto :eof
)
:runapp
echo Running...
:: Run built exe
cd build
%APP_NAME%
cd ..