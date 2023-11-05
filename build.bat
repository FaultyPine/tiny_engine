@echo off

cls

set pythoncmd=python
WHERE %pythoncmd% >nul 2>nul
IF NOT %ERRORLEVEL% == 0 (
    echo Python not found, make sure it is installed, or change the python command in this batch file (build.bat)
)

pushd tools\build_tools
call "vsdev.bat"
popd


:: build types
pushd types
echo =========== TYPES ===========
%pythoncmd% build.py %*
popd

:: build engine
pushd engine
echo =========== Engine ===========
%pythoncmd% build.py %*
popd

:: build editor
pushd editor
echo =========== Editor ===========
%pythoncmd% build.py %*
popd

:: build game
pushd game
echo =========== Game ===========
%pythoncmd% build.py %*
popd
