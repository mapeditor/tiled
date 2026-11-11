@echo off
rem Registers the x64-hosted ARM64 cross compiler with qbs, since
rem setup-toolchains --detect would pick the x64 toolchain instead.
for /d %%d in ("%ProgramFiles%\Microsoft Visual Studio\2022\*\VC\Tools\MSVC\*") do if exist "%%d\bin\Hostx64\ARM64\cl.exe" set MSVC_DIR=%%d
if not defined MSVC_DIR (
    echo No ARM64 MSVC toolchain found
    exit /b 1
)
echo Using ARM64 compiler: %MSVC_DIR%\bin\Hostx64\ARM64\cl.exe
qbs setup-toolchains "%MSVC_DIR%\bin\Hostx64\ARM64\cl.exe" msvc-arm64 || exit /b 1
qbs config defaultProfile msvc-arm64
