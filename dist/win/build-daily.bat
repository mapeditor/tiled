@rem  TILED DAILY BUILD SCRIPT
@echo off

rem The following assumes US date format!

set VERSION=%DATE:~10,4%-%DATE:~4,2%-%DATE:~7,2%

set TILED_SOURCE_DIR=D:\Projects\tiled
set TILED_BUILD_DIR=D:\Builds\
set QTDIR=C:\Qt\5.5\msvc2013_64
set ARCH=32
set MAKE=C:\Qt\Tools\QtCreator\bin\jom.exe
set GIT="C:\Users\Zo0MER\AppData\Local\Programs\Git\bin\git.exe"
set SCP="C:\Users\Zo0MER\AppData\Local\Programs\Git\usr\bin\scp.exe"
set DESTINATION=bjorn@files.mapeditor.org:public_html/files.mapeditor.org/public/daily/

pushd %TILED_SOURCE_DIR%
%GIT% fetch
%GIT% diff --quiet origin/master
if %ERRORLEVEL% == 0 (
    echo No change, nothing to do.
    popd
    goto done
) else if %ERRORLEVEL% == 1 (
   %GIT% reset --soft origin/master
)
popd

FOR /F "tokens=*" %%i in ('%GIT% describe') do SET COMMITNOW=%%i

echo Building Tiled daily %VERSION%... (from %COMMITNOW%)

call %QTDIR%\bin\qtenv2.bat
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86

rem This allows the executable to run on Windows XP
set LINK=/SUBSYSTEM:WINDOWS,5.01

mkdir %TILED_BUILD_DIR%
pushd %TILED_BUILD_DIR%
qmake.exe -r %TILED_SOURCE_DIR%\tiled.pro "CONFIG+=release" "QMAKE_CXXFLAGS+=-DTILED_VERSION=%COMMITNOW%"
%MAKE%
popd

echo Building Installer...
pushd %TILED_SOURCE_DIR%\dist\win
C:\PROGRA~1\NSIS\NSISmakensis.exe tiled-vs2013.nsi
