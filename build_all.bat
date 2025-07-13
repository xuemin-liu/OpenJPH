@echo off
echo ===============================================
echo OpenJPH - Build All Architectures
echo ===============================================
echo.
echo This script will:
echo 1. Generate Visual Studio solutions for both architectures
echo 2. Build OpenJPH for both architectures:
echo    - x64 (64-bit)
echo    - x86 (32-bit)
echo 3. Build both Debug and Release configurations
echo.

set BUILD_FAILED=0

echo ===============================================
echo Step 1: Generating Solutions
echo ===============================================
echo.

echo Generating Visual Studio solutions for all architectures...
call "%~dp0generate_solution.bat"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to generate solutions!
    set BUILD_FAILED=1
    goto :summary
)

echo.
echo ===============================================
echo Step 2: Building x64 Architecture
echo ===============================================
echo.

echo --- x64 Release Build ---
call "%~dp0build_release_x64.bat"
if %ERRORLEVEL% NEQ 0 set BUILD_FAILED=1

echo.
echo --- x64 Debug Build ---
call "%~dp0build_debug_x64.bat" 
if %ERRORLEVEL% NEQ 0 set BUILD_FAILED=1

echo.
echo ===============================================
echo Step 3: Building x86 Architecture  
echo ===============================================
echo.

echo --- x86 Release Build ---
call "%~dp0build_release_x86.bat"
if %ERRORLEVEL% NEQ 0 set BUILD_FAILED=1

echo.
echo --- x86 Debug Build ---
call "%~dp0build_debug_x86.bat"
if %ERRORLEVEL% NEQ 0 set BUILD_FAILED=1

echo.
:summary
echo ===============================================
echo Build Summary
echo ===============================================

if %BUILD_FAILED% EQU 0 (
    echo [SUCCESS] ALL BUILDS COMPLETED SUCCESSFULLY!
    echo.
    echo Built configurations:
    echo - x64 Release: build_x64\src\core\Release\openjph.0.21.lib
    echo - x64 Debug:   build_x64\src\core\Debug\openjph.0.21.lib  
    echo - x86 Release: build_x86\src\core\Release\openjph.0.21.lib
    echo - x86 Debug:   build_x86\src\core\Debug\openjph.0.21.lib
    echo.
    echo Wrapper DLLs:
    echo - x64 Release: build_x64\src\apps\ojph_wrapper\Release\ojph_wrapper.dll
    echo - x64 Debug:   build_x64\src\apps\ojph_wrapper\Debug\ojph_wrapperd.dll
    echo - x86 Release: build_x86\src\apps\ojph_wrapper\Release\ojph_wrapper.dll  
    echo - x86 Debug:   build_x86\src\apps\ojph_wrapper\Debug\ojph_wrapperd.dll
) else (
    echo [ERROR] Some builds failed! Check the output above for details.
)
