@echo off

cls

pushd tools\build
call "vsdev.bat"
popd

if ["%1"]==["types"] (
    :: build types
    pushd types
    echo =========== TYPES ===========
    call build.bat && run.bat
    popd
)

:: build engine
pushd engine
echo =========== Engine ===========
python build.py %*
popd

:: build editor
pushd editor

popd

:: build game
pushd game
echo =========== Game ===========
python build.py %*
popd
