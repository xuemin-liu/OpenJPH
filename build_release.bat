@echo off
echo Building OpenJPH in Release configuration (Static Library with /MT)...
cd /d "%~dp0\build"
cmake --build . --config Release
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Release build completed successfully!
    echo ===================================
    echo.
    echo Built files (Static Library):
    echo - Library: build\src\core\Release\openjph.0.21.lib
    echo - Wrapper DLL: build\src\apps\ojph_wrapper\Release\ojph_wrapper.dll
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
