@echo off

call "vsdev.bat"
echo Building types metaprogram...
REM /TP forces C++ compilation mode despite .c extension
cl type_metadata.cpp /Zi /TP /std:c++17 /EHsc /nologo