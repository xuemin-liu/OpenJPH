@echo off
echo ===============================================
echo OpenJPH - Advanced Solution Generator
echo ===============================================
echo.
echo Select Visual Studio version:
echo 1. Visual Studio 2022 (default)
echo 2. Visual Studio 2019
echo 3. Visual Studio 2017
echo 4. Cancel
echo.
set /p choice="Enter your choice (1-4): "

if "%choice%"=="1" (
    set VS_GENERATOR="Visual Studio 17 2022"
    set VS_NAME=2022
) else if "%choice%"=="2" (
    set VS_GENERATOR="Visual Studio 16 2019"
    set VS_NAME=2019
) else if "%choice%"=="3" (
    set VS_GENERATOR="Visual Studio 15 2017"
    set VS_NAME=2017
) else if "%choice%"=="4" (
    echo Operation cancelled.
    pause
    exit /b 0
) else (
    echo Invalid choice, using Visual Studio 2022 as default.
    set VS_GENERATOR="Visual Studio 17 2022"
    set VS_NAME=2022
)

echo.
echo Configuration options:
echo 1. Static library with static runtime (/MT, /MTd) - Recommended
echo 2. Shared library with dynamic runtime (/MD, /MDd)
echo.
set /p lib_choice="Enter your choice (1-2): "

if "%lib_choice%"=="2" (
    set BUILD_SHARED=ON
    set LIB_TYPE=Shared DLL
    set RUNTIME_TYPE=Dynamic runtime
) else (
    set BUILD_SHARED=OFF
    set LIB_TYPE=Static library
    set RUNTIME_TYPE=Static runtime
)

echo.
echo ===============================================
echo Generating solution with:
echo - Visual Studio: %VS_NAME%
echo - Library type: %LIB_TYPE%
echo - Runtime: %RUNTIME_TYPE%
echo ===============================================
echo.

cd /d "%~dp0\build"

echo Step 1: Cleaning existing files...
echo ----------------------------------

:: Force remove all Visual Studio files
powershell -Command "Remove-Item '*.sln', '*.vcxproj*' -ErrorAction SilentlyContinue"
powershell -Command "Get-ChildItem -Path . -Recurse -Name '*.vcxproj*' | Remove-Item -Force -ErrorAction SilentlyContinue"

:: Remove CMake files
if exist "CMakeCache.txt" del /Q "CMakeCache.txt" 2>nul
if exist "CMakeFiles\" rmdir /S /Q "CMakeFiles" 2>nul

echo.
echo Step 2: Generating CMake configuration...
echo -----------------------------------------

:: Generate with selected options
cmake .. -G %VS_GENERATOR% -A x64 -DBUILD_SHARED_LIBS=%BUILD_SHARED%

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===============================================
    echo SUCCESS: Visual Studio %VS_NAME% solution generated!
    echo ===============================================
    echo.
    echo Configuration:
    echo - Library type: %LIB_TYPE%
    echo - Runtime linking: %RUNTIME_TYPE%
    echo - Architecture: x64
    echo - SIMD optimizations: Enabled
    echo.
    echo Solution file: build\openjph.sln
    echo.
) else (
    echo.
    echo ===============================================
    echo FAILED: Solution generation failed!
    echo ===============================================
)

pause
