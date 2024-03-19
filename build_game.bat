@echo off

call "prep.bat"


pushd game
echo =========== Game ===========
%pythoncmd% build.py %*
popd