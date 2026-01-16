@echo off
setlocal

set BUILD_DIR=build
set SRC_DIR=src
set LIMINE_DIR=%BUILD_DIR%\limine
set ISO_IMAGE=%BUILD_DIR%\paradoxos.iso

echo [INFO] checking dependencies...
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] gcc not found! Please install MinGW or similar.
    exit /b 1
)
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] git not found!
    exit /b 1
)

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo [INFO] Downloading Limine...
if not exist %LIMINE_DIR% (
    git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1 %LIMINE_DIR%
)

echo [INFO] Compiling Kernel...
if not exist %BUILD_DIR%\src\kernel mkdir %BUILD_DIR%\src\kernel
set OBJ_FILES=
for /R src\kernel %%f in (*.c) do (
    echo [INFO] Compiling %%f...
    gcc -g -m64 -march=x86-64 -ffreestanding -fno-builtin -nostdlib -mno-red-zone -mgeneral-regs-only -Wall -Wextra -I src/boot -I src/kernel -c %%f -o %BUILD_DIR%\src\kernel\%%~nf.o
    if errorlevel 1 exit /b 1
    call set OBJ_FILES=%%OBJ_FILES%% %BUILD_DIR%\src\kernel\%%~nf.o
)

echo [INFO] Linking Kernel...
ld -m i386pep -nostdlib --image-base 0xffffffff80000000 -e _start -T src/kernel/linker.ld -o %BUILD_DIR%/kernel.elf %OBJ_FILES%

:: Note: MinGW ld might need -m i386pep for 64-bit output on Windows, but let's try standard elf_x86_64 if available or default. 
:: Standard ld on Windows (MinGW) supports i386pep. We strictly want ELF for Limine. 
:: We might need to force objcopy if ld only outputs PE.
:: Let's check if we can output elf64-x86-64.
:: ELF linking attempt removed as it is not supported by this linker.

echo [INFO] Checking for xorriso...
if exist scripts\xorriso.exe (
    set XORRISO_CMD=scripts\xorriso.exe
    echo [INFO] Using local xorriso.exe
) else (
    where xorriso >nul 2>nul
    if %errorlevel% neq 0 (
        echo [ERROR] 'xorriso' is missing! Cannot build ISO.
        echo [TIP] Install it via 'winget install GNU.Xorriso' or strictly use the 'boot/limine' deploy method on a USB stick.
        echo [INFO] The kernel binary is available at: %BUILD_DIR%\kernel.elf
        exit /b 1
    )
    set XORRISO_CMD=xorriso
)

echo [INFO] Building ISO...
mkdir %BUILD_DIR%\iso_root 2>nul
copy %BUILD_DIR%\kernel.elf %BUILD_DIR%\iso_root\ >nul
copy limine.conf %BUILD_DIR%\iso_root\ >nul
copy %LIMINE_DIR%\limine-bios.sys %BUILD_DIR%\iso_root\ >nul
copy %LIMINE_DIR%\limine-bios-cd.bin %BUILD_DIR%\iso_root\ >nul
copy %LIMINE_DIR%\limine-uefi-cd.bin %BUILD_DIR%\iso_root\ >nul

%XORRISO_CMD% -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label %BUILD_DIR%/iso_root -o %ISO_IMAGE%


echo [INFO] Patching ISO for BIOS...
%LIMINE_DIR%\limine.exe bios-install %ISO_IMAGE% 2>nul
if %errorlevel% neq 0 (
    echo [WARNING] Could not run limine.exe to patch BIOS boot. UEFI should still work.
)

echo [SUCCESS] ISO built at %ISO_IMAGE%
