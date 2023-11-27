@echo off

set pythoncmd=python
WHERE %pythoncmd% >nul 2>nul
IF NOT %ERRORLEVEL% == 0 (
    echo Python not found, make sure it is installed, or change the python command in this file (run.bat)
)

if ["%1"]==["game"] (
    pushd game
    %pythoncmd% build.py run
    popd
)

if ["%1"]==["editor"] (
    pushd editor
    %pythoncmd% build.py run
    popd
)

if ["%1"]==[""] (
    pushd game
    %pythoncmd% build.py run
    popd
)
