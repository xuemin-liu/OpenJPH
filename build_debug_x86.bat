@echo off
echo Building OpenJPH in Debug configuration (Static Library with /MTd - x86)...
cd /d "%~dp0\build_x86"
cmake --build . --config Debug
if errorlevel 1 goto :failed

echo.
echo ===================================
echo x86 Debug build completed successfully!
echo ===================================
echo.
echo Built files (Static Library - 32-bit):
echo - Library: build_x86\src\core\Debug\openjph.0.21.lib
echo - Wrapper DLL: build_x86\src\apps\ojph_wrapper\Debug\ojph_wrapperd.dll
echo - Compress: build_x86\src\apps\ojph_compress\Debug\ojph_compress.exe
echo - Expand: build_x86\src\apps\ojph_expand\Debug\ojph_expand.exe
echo.
echo Runtime: Static linking (/MTd)
echo Architecture: 32-bit (x86)
goto :end

:failed
echo.
echo ===================================
echo x86 Debug build FAILED!
echo ===================================
echo Please check the output above for errors.

:end
