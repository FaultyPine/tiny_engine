@echo off

call "prep.bat"

pushd types
echo =========== TYPES ===========
%pythoncmd% build.py %*
popd