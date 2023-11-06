@echo off

call "prep.bat"


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
