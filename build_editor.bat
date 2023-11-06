@echo off

call "prep.bat"

pushd editor
echo =========== Editor ===========
%pythoncmd% build.py %*
popd