@echo off
echo ========================================
echo ParadoxOS Simple Build Attempt
echo ========================================
echo.

echo Attempting to build with available Windows tools...
echo This may not create a fully functional kernel, but let's try!
echo.

:: Check what we have
echo Checking available tools:
where gcc >nul 2>&1 && echo ✓ GCC found || echo ✗ GCC not found
where ld >nul 2>&1 && echo ✓ LD found || echo ✗ LD not found  
where nasm >nul 2>&1 && echo ✓ NASM found || echo ✗ NASM not found
where mingw32-make >nul 2>&1 && echo ✓ Make found || echo ✗ Make not found

echo.
echo Attempting build with regular GCC (experimental)...
echo.

:: Try to build with regular tools
mingw32-make clean
mingw32-make CC=gcc LD=ld

if %errorlevel% equ 0 (
    echo.
    echo ✓ Build succeeded with regular GCC!
    echo ⚠ Warning: This may not be a proper kernel binary
    echo.
    
    :: Create basic ISO structure
    if not exist "build" mkdir build
    if not exist "build\iso" mkdir build\iso
    if not exist "build\iso\boot" mkdir build\iso\boot
    
    copy paradox.elf build\iso\boot\ >nul 2>&1
    copy limine.cfg build\iso\boot\ >nul 2>&1
    
    echo ✓ Basic ISO structure created in build\iso\
    echo.
    echo Next steps:
    echo 1. Download Limine bootloader files
    echo 2. Use ISO creation software like ImgBurn
    echo 3. Test in VirtualBox
    
) else (
    echo.
    echo ❌ Build failed with regular GCC
    echo.
    echo You need proper cross-compiler tools.
    echo Run setup-windows-native.bat for instructions.
)

echo.
pause