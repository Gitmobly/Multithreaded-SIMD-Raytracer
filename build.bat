@echo off
setlocal EnableDelayedExpansion

set BUILD_DIR=build
set SRC_DIR=src

set CONFIG=debug
if /I "%~1"=="-release" set CONFIG=release

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

set CLFLAGS=/std:c++20 /W4 /EHsc /clang:-Wno-unused-function /clang:-Wno-missing-braces /clang:-Wno-missing-field-initializers /clang:-Wno-deprecated-declarations

if /I "%CONFIG%"=="release" (
    set CONFIGFLAGS=/DNDEBUG /O2 /Zi /arch:AVX2
) else (
    set CONFIGFLAGS=/DDEBUG_BUILD /DTESTS_ON /Zi /Od
)

set SOURCES=
for /R "%SRC_DIR%" %%f in (*.cpp) do (
    set SOURCES=!SOURCES! "%%f"
)

echo Building %CONFIG%...

clang-cl ^
    %CLFLAGS% ^
    !CONFIGFLAGS! ^
    /I%SRC_DIR% ^
    !SOURCES! ^
    /Fe:%BUILD_DIR%\main.exe ^
    /Fo:%BUILD_DIR%\

if errorlevel 1 exit /b %errorlevel%
