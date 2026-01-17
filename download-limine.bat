@echo off
echo ========================================
echo Download Limine Bootloader
echo ========================================
echo.

echo Downloading Limine bootloader files...
echo.

:: Create limine directory
if not exist "limine" mkdir limine

:: Download using PowerShell (available on all modern Windows)
echo Downloading Limine release...
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/limine-bootloader/limine/releases/download/v4.20231210.0/limine-4.20231210.0.tar.xz' -OutFile 'limine-release.tar.xz'}"

if exist "limine-release.tar.xz" (
    echo ✓ Limine downloaded successfully
    echo.
    echo To extract and use:
    echo 1. Install 7-Zip or WinRAR
    echo 2. Extract limine-release.tar.xz
    echo 3. Copy these files to limine\ folder:
    echo    - limine-cd.bin
    echo    - limine-cd-efi.bin  
    echo    - limine.sys
    echo.
) else (
    echo ❌ Download failed
    echo.
    echo Manual download:
    echo 1. Go to: https://github.com/limine-bootloader/limine/releases
    echo 2. Download: limine-4.20231210.0.tar.xz
    echo 3. Extract and copy the files mentioned above
)

pause