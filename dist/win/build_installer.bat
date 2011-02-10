@echo off

setlocal EnableDelayedExpansion

REM Setting environment variables
if not defined MINGW     (set MINGW=C:\MinGW)
if not defined QTDIR     (set QTDIR=C:\Qt\4.7.1)
if not defined ARCH      (set ARCH=32)
if not defined VERSION   (set /p VERSION=<..\..\build\VERSION-FILE)

REM Starting the compilation process
echo "BUILDING INSTALLER"
makensis.exe tiled.nsi

endlocal
