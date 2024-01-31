@echo off

call "prep.bat"


:: build types
pushd types
echo =========== TYPES ===========
%pythoncmd% build.py %*
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && popd && exit /b)
popd

:: build engine
pushd engine
echo =========== Engine ===========
%pythoncmd% build.py %*
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && popd && exit /b)
popd

:: build game
pushd movement_proj
echo =========== Game ===========
%pythoncmd% build.py %*
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && popd && exit /b)
popd

:: build editor
pushd editor
echo =========== Editor ===========
%pythoncmd% build.py %*
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && popd && exit /b)
popd

