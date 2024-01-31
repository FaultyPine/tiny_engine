@echo off

call "prep.bat"


pushd movement_proj
echo =========== Game ===========
%pythoncmd% build.py %*
popd