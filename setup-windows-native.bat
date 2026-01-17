@echo off
echo ========================================
echo ParadoxOS Windows Native Setup
echo ========================================
echo.

echo This script will help you set up the build environment for Windows.
echo.

echo Step 1: Download Pre-built Cross-Compiler
echo ==========================================
echo.
echo Please download the pre-built x86_64-elf toolchain:
echo.
echo 1. Go to: https://github.com/lordmilko/i686-elf-tools/releases
echo 2. Download: x86_64-elf-tools-windows.zip
echo 3. Extract to: C:\cross-compiler\
echo 4. The folder should contain: bin\, lib\, include\, etc.
echo.
pause

echo.
echo Step 2: Download NASM Assembler
echo ================================
echo.
echo Please download NASM:
echo.
echo 1. Go to: https://www.nasm.us/pub/nasm/releasebuilds/
echo 2. Download the latest Windows version (e.g., nasm-2.16.01-win64.zip)
echo 3. Extract to: C:\nasm\
echo 4. The folder should contain: nasm.exe, ndisasm.exe
echo.
pause

echo.
echo Step 3: Download xorriso (for ISO creation)
echo ============================================
echo.
echo Please download xorriso:
echo.
echo 1. Go to: https://www.gnu.org/software/xorriso/
echo 2. Download Windows binary or use cdrtools
echo 3. Extract to: C:\xorriso\
echo.
echo Alternatively, we can create the ISO structure and you can use
echo other tools like ImgBurn or similar to create the final ISO.
echo.
pause

echo.
echo Step 4: Add to Windows PATH
echo ============================
echo.
echo Add these directories to your Windows PATH:
echo - C:\cross-compiler\bin
echo - C:\nasm
echo - C:\xorriso
echo.
echo To add to PATH:
echo 1. Press Win+R, type: sysdm.cpl
echo 2. Click "Environment Variables"
echo 3. Edit "Path" in System Variables
echo 4. Add the directories above
echo 5. Click OK and restart Command Prompt
echo.
pause

echo.
echo Step 5: Verify Installation
echo ============================
echo.
echo After adding to PATH, verify with these commands:
echo.
echo x86_64-elf-gcc --version
echo nasm -version
echo.
echo If these work, you can build ParadoxOS with:
echo make -f Makefile.cross
echo.

echo Setup guide complete!
echo Run this script again if you need to see the instructions.
pause