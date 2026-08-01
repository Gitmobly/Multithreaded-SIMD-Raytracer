@echo off
setlocal EnableDelayedExpansion

set BUILD_DIR=build
set SRC_DIR=src

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

set CLFLAGS=/std:c++20 /W4 /EHsc /clang:-Wno-unused-function /clang:-Wno-missing-braces /clang:-Wno-missing-field-initializers /clang:-Wno-deprecated-declarations
set DEBUG=/DDEBUG_BUILD /DTESTS_ON /Zi /Od

set SOURCES=

for /R "%SRC_DIR%" %%f in (*.cpp) do (
    set SOURCES=!SOURCES! "%%f"
)

clang-cl ^
    %CLFLAGS% ^
    %DEBUG% ^
    /I"%SRC_DIR%" ^
    %SOURCES% ^
    /Fe:"%BUILD_DIR%\main.exe"

if errorlevel 1 exit /b %errorlevel%
