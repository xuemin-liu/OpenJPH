@echo off
echo Building OpenJPH in Release configuration (Static Library with /MT)...
cd /d "%~dp0\build"

echo Checking if CMake reconfiguration is needed...

:: Check if solution file exists and if CMakeLists.txt is newer
if not exist "openjph.sln" (
    echo Solution file missing - regenerating...
    goto :regenerate
)

:: Compare file dates (this is a simplified check)
for %%i in ("..\CMakeLists.txt") do set CMAKE_TIME=%%~ti
for %%i in ("openjph.sln") do set SLN_TIME=%%~ti

echo CMakeLists.txt time: %CMAKE_TIME%
echo Solution time: %SLN_TIME%

:: If CMakeCache doesn't exist, force regeneration
if not exist "CMakeCache.txt" (
    echo CMake cache missing - regenerating...
    goto :regenerate
)

goto :build

:regenerate
echo.
echo ===================================
echo Regenerating CMake configuration and Visual Studio solution...
echo ===================================
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=OFF
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ===================================
    echo CMake configuration FAILED!
    echo ===================================
    pause
    exit /b 1
)

:build
echo.
echo ===================================
echo Building Release configuration...
echo ===================================
cmake --build . --config Release
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Release build completed successfully!
    echo ===================================
    echo.
    echo Built files (Static Library):
    echo - Library: build\src\core\Release\openjph.0.21.lib
    echo - Compress: build\src\apps\ojph_compress\Release\ojph_compress.exe
    echo - Expand: build\src\apps\ojph_expand\Release\ojph_expand.exe
    echo.
    echo Runtime: Static linking (/MT)
) else (
    echo.
    echo ===================================
    echo Release build FAILED!
    echo ===================================
)
pause
