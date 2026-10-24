@echo off
rem Builds and installs the qaseprite image format plugin. Called by
rem appveyor.yml when qaseprite.dll is not already in the Qt cache.
git clone --depth 1 --recurse-submodules --shallow-submodules --branch %QASEPRITE_VERSION% https://github.com/mapeditor/qaseprite.git || exit /b 1
pushd qaseprite\aseprite\laf || exit /b 1
patch -p1 < ..\..\laf-msvc-dynamic-runtime.patch || exit /b 1
popd
pushd qaseprite || exit /b 1
if defined MINGW (
    cmake -B build -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
) else (
    cmake -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
)
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel || exit /b 1
cmake --install build --config Release || exit /b 1
popd
