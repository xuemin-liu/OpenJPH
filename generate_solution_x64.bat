@echo off
echo ===============================================
echo OpenJPH - Generate Visual Studio Solution (x64)
echo ===============================================
echo.
echo This will:
echo - Delete existing Visual Studio solution and project files
echo - Regenerate CMake configuration from scratch for 64-bit build
echo - Create new Visual Studio 2022 solution with static library settings
echo.

:: Create build_x64 directory if it doesn't exist
if not exist "%~dp0\build_x64" (
    echo Creating build_x64 directory...
    mkdir "%~dp0\build_x64"
)

cd /d "%~dp0\build_x64"

echo Step 1: Cleaning existing Visual Studio files...
echo -----------------------------------------------

:: Remove Visual Studio solution and project files
if exist "*.sln" (
    echo Removing solution files...
    del /Q "*.sln" 2>nul
)

if exist "*.vcxproj*" (
    echo Removing project files...
    del /Q "*.vcxproj*" 2>nul
)

:: Remove project files from subdirectories
if exist "src\" (
    echo Removing project files from subdirectories...
    for /r src %%f in (*.vcxproj*) do del /Q "%%f" 2>nul
)

:: Remove CMake cache and generated files
echo Removing CMake cache and generated files...
if exist "CMakeCache.txt" del /Q "CMakeCache.txt" 2>nul
if exist "cmake_install.cmake" del /Q "cmake_install.cmake" 2>nul
if exist "ALL_BUILD.vcxproj*" del /Q "ALL_BUILD.vcxproj*" 2>nul
if exist "ZERO_CHECK.vcxproj*" del /Q "ZERO_CHECK.vcxproj*" 2>nul
if exist "INSTALL.vcxproj*" del /Q "INSTALL.vcxproj*" 2>nul

:: Remove CMakeFiles directory (contains cached configuration)
if exist "CMakeFiles\" (
    echo Removing CMakeFiles directory...
    rmdir /S /Q "CMakeFiles" 2>nul
)

echo.
echo Step 2: Generating new CMake configuration and Visual Studio solution (x64)...
echo ----------------------------------------------------------------------

:: Generate new CMake configuration with our specific settings for x64
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=OFF

:: Check if solution file was created successfully
if exist "openjph.sln" (
    echo.
    echo ===============================================
    echo SUCCESS: Visual Studio x64 solution generated!
    echo ===============================================
    echo.
    echo Generated files:
    echo - Solution: build_x64\openjph.sln
    echo - Core library project: build_x64\src\core\openjph.vcxproj  
    echo - Applications: build_x64\src\apps\*\*.vcxproj
    echo.
    echo Configuration:
    echo - Static library (openjph.0.21.lib)
    echo - 64-bit (x64) architecture
    echo - Static runtime linking (/MT for Release, /MTd for Debug)
    echo - SIMD optimizations enabled
    echo.
    echo You can now:
    echo 1. Open build_x64\openjph.sln in Visual Studio 2022
    echo 2. Use build_release_x64.bat or build_debug_x64.bat to compile
    echo 3. Build directly with: cmake --build . --config Release
    echo.
    goto :end
)

echo.
echo ===============================================
echo FAILED: Visual Studio x64 solution not created!
echo ===============================================
echo.
echo Please check the CMake output above for errors.
echo Common issues:
echo - Visual Studio 2022 not installed
echo - CMake not found in PATH
echo - Invalid CMakeLists.txt syntax
echo.

:end
