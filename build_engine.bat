@echo off

call "prep.bat"

pushd engine
echo =========== Engine ===========
%pythoncmd% build.py %*
popd