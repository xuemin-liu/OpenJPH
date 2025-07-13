@echo off
echo ===============================================
echo OpenJPH - Generate Visual Studio Solutions (All Architectures)
echo ===============================================
echo.
echo This will generate solutions for both architectures:
echo - x64 (64-bit)
echo - x86 (32-bit)
echo.

echo Generating x64 solution...
call "%~dp0generate_solution_x64.bat"

echo.
echo Generating x86 solution...
call "%~dp0generate_solution_x86.bat"

echo.
echo ===============================================
echo All solutions generated!
echo ===============================================
echo.
echo Available solutions:
echo - x64: build_x64\openjph.sln
echo - x86: build_x86\openjph.sln
