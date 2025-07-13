@echo off
echo Building OpenJPH in Debug configuration (Static Library with /MTd)...
cd /d "%~dp0\build"
cmake --build . --config Debug
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Debug build completed successfully!
    echo ===================================
    echo.
    echo Built files (Static Library):
    echo - Library: build\src\core\Debug\openjph.0.21.lib
    echo - Wrapper DLL: build\src\apps\ojph_wrapper\Debug\ojph_wrapper.dll
    echo - Compress: build\src\apps\ojph_compress\Debug\ojph_compress.exe
    echo - Expand: build\src\apps\ojph_expand\Debug\ojph_expand.exe
    echo.
    echo Runtime: Static linking (/MTd)
) else (
    echo.
    echo ===================================
    echo Debug build FAILED!
    echo ===================================
)
pause
