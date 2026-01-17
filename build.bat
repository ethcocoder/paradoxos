@echo off
echo ========================================
echo ParadoxOS Build Script
echo ========================================
echo.

echo Checking build environment...

:: Check for GCC
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: GCC not found!
    echo Please install MinGW-w64 or similar
    goto :error
)
echo ✓ GCC found

:: Check for LD
where ld >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: LD not found!
    goto :error
)
echo ✓ LD found

:: Check for cross-compiler
where x86_64-elf-gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: x86_64-elf-gcc not found!
    echo Using regular GCC (may not work for kernel)
    set CC=gcc
    set LD=ld
) else (
    echo ✓ x86_64-elf-gcc found
    set CC=x86_64-elf-gcc
    set LD=x86_64-elf-ld
)

echo.
echo Building ParadoxOS kernel...
echo.

:: Clean previous build
echo Cleaning previous build...
mingw32-make clean

:: Build kernel
echo Building kernel...
mingw32-make CC=%CC% LD=%LD%

if %errorlevel% neq 0 (
    echo.
    echo BUILD FAILED!
    echo.
    echo This is likely because we need a proper x86_64 cross-compiler.
    echo.
    echo To build ParadoxOS properly, you need:
    echo 1. x86_64-elf-gcc cross-compiler
    echo 2. x86_64-elf-ld linker
    echo 3. NASM assembler
    echo.
    echo You can get these from:
    echo - https://github.com/lordmilko/i686-elf-tools (for Windows)
    echo - Or use WSL with: sudo apt install gcc-multilib nasm
    echo.
    goto :error
)

echo.
echo ✓ Kernel built successfully!
echo.

:: Check if we can create ISO structure
echo Setting up ISO structure...
if not exist "build" mkdir build
if not exist "build\iso" mkdir build\iso
if not exist "build\iso\boot" mkdir build\iso\boot

copy paradox.elf build\iso\boot\ >nul
copy limine.cfg build\iso\boot\ >nul

echo ✓ ISO structure created in build\iso\

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Kernel: paradox.elf
echo ISO structure: build\iso\
echo.
echo To create a bootable ISO, you need:
echo 1. Download Limine bootloader from:
echo    https://github.com/limine-bootloader/limine/releases
echo 2. Extract limine-cd.bin and limine-cd-efi.bin to limine\ folder
echo 3. Install xorriso for ISO creation
echo 4. Run the ISO creation command
echo.
echo To test in QEMU:
echo qemu-system-x86_64 -kernel paradox.elf -m 256M
echo.
goto :end

:error
echo.
echo BUILD FAILED!
echo See messages above for details.
echo.
pause
exit /b 1

:end
echo Build completed successfully!
pause