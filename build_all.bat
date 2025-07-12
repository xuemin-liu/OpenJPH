@echo off
echo Building OpenJPH in both Debug and Release configurations (Static Libraries)...
echo.

echo ===================================
echo Building Debug configuration (/MTd)...
echo ===================================
cd /d "%~dp0\build"
cmake --build . --config Debug
set DEBUG_RESULT=%ERRORLEVEL%

echo.
echo ===================================
echo Building Release configuration (/MT)...
echo ===================================
cmake --build . --config Release
set RELEASE_RESULT=%ERRORLEVEL%

echo.
echo ===================================
echo Build Summary
echo ===================================
if %DEBUG_RESULT% EQU 0 (
    echo Debug build: SUCCESS
) else (
    echo Debug build: FAILED
)

if %RELEASE_RESULT% EQU 0 (
    echo Release build: SUCCESS
) else (
    echo Release build: FAILED
)

echo.
if %DEBUG_RESULT% EQU 0 if %RELEASE_RESULT% EQU 0 (
    echo All builds completed successfully!
    echo.
    echo Built files available (Static Libraries):
    echo - Debug: build\src\core\Debug\openjph.0.21.lib (/MTd runtime)
    echo - Release: build\src\core\Release\openjph.0.21.lib (/MT runtime)
    echo - Applications in: build\src\apps\*\Debug\ and build\src\apps\*\Release\
) else (
    echo One or more builds failed. Check the output above for details.
)
echo ===================================
pause
