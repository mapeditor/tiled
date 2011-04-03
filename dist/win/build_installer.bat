@echo off

setlocal EnableDelayedExpansion

:: Check for MinGW
if not defined MINGW (
    :get_mingw_path
    call :get_path MinGW MINGW
)

call :check_path "!MINGW!" mingw32-make.exe MinGW MAKE
if not defined MAKE (
    call :check_path "!MINGW!" make.exe MinGW MAKE
    if not defined MAKE (
        echo Failed to detect MinGW
        goto get_mingw_path
    )
)

:: Check for Qt
if not defined QTDIR (
    :get_qt_path
    call:get_path Qt QTDIR
)

call :check_path "!QTDIR!" qmake.exe Qt QMAKE
if not defined QMAKE (
    echo Failed to detect Qt
    goto get_qt_path
)

:check_arch
if not defined ARCH (
	set /P ARCH="Please either enter Architecture (32 or 64) or press enter for default (32)":
	if not defined ARCH (
		set ARCH=32
		echo Set to default: 32
	)
)

if not defined VERSION (
	if exist ..\..\build\VERSION_FILE (
		echo "file exists"
		set /P VERSION=<..\..\VERSION_FILE
	)

	if not defined VERSION (
		:get_version_number
		set /P VERSION=Please enter the version number to be used:
		if not defined VERSION (
			echo You didn't enter a valid version number
			goto get_version_number
		)
	)
)

:: Starting the compilation process
echo "BUILDING INSTALLER"
makensis.exe tiled.nsi

endlocal
goto:eof

::--------------------------------------------------------
::-- Function section starts below here
::--------------------------------------------------------
:get_path
setlocal
set /P EXE_PATH=Please enter path to %~1:
if not exist "%EXE_PATH%" (
    echo The directory you entered does not exist!
    goto get_path
) else (
    endlocal
    set "%~2=%EXE_PATH%"
)
goto:eof

:check_path
if exist %~1\bin\%~2 (
    echo Successfully detected %~3!
    set "%~4=%~2"
) else ( set "%~4=" )
goto:eof
