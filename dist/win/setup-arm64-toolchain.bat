@echo off
rem Registers the x64-hosted ARM64 cross compiler with qbs, since
rem setup-toolchains --detect would pick the x64 toolchain instead.
for /f "usebackq delims=" %%i in (`""%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -find VC\Tools\MSVC\*\bin\Hostx64\ARM64\cl.exe"`) do set ARM64_CL=%%i
if not defined ARM64_CL (
    echo No ARM64 MSVC toolchain found
    exit /b 1
)
echo Using ARM64 compiler: %ARM64_CL%
qbs setup-toolchains "%ARM64_CL%" msvc-arm64 || exit /b 1
qbs config defaultProfile msvc-arm64
