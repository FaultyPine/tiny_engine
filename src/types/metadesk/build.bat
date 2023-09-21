@echo off

call "vsdev.bat"
cl type_metadata.c
type_metadata.exe types.mdesk