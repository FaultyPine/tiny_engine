@echo off

call "vsdev.bat"
REM /TP forces C++ compilation mode despite .c extension
cl type_metadata.c /Zi /TP /std:c++17 /EHsc