@echo off
echo Configuring and Building OpenJPH in Release configuration (Static Library with /MT)...
cd /d "%~dp0\build"

echo ===================================
echo Step 1: Regenerating CMake configuration and Visual Studio solution...
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

echo.
echo ===================================
echo Step 2: Building Release configuration...
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
