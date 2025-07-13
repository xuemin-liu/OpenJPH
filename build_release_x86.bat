@echo off
echo Building OpenJPH in Release configuration (Static Library with /MT - x86)...
cd /d "%~dp0\build_x86"
cmake --build . --config Release
if errorlevel 1 goto :failed

echo.
echo ===================================
echo x86 Release build completed successfully!
echo ===================================
echo.
echo Built files (Static Library - 32-bit):
echo - Library: build_x86\src\core\Release\openjph.0.21.lib
echo - Wrapper DLL: build_x86\src\apps\ojph_wrapper\Release\ojph_wrapper.dll
echo - Compress: build_x86\src\apps\ojph_compress\Release\ojph_compress.exe
echo - Expand: build_x86\src\apps\ojph_expand\Release\ojph_expand.exe
echo.
echo Runtime: Static linking (/MT)
echo Architecture: 32-bit (x86)
goto :end

:failed
echo.
echo ===================================
echo x86 Release build FAILED!
echo ===================================
echo Please check the output above for errors.

:end
