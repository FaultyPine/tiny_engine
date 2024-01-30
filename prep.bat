@echo off


cls

set pythoncmd=python
WHERE %pythoncmd% >nul 2>nul
IF NOT %ERRORLEVEL% == 0 (
    echo Python not found, make sure it is installed, or change the python command in this batch file (prep.bat)
)