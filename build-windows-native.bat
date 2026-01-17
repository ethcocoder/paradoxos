@echo off
echo ========================================
echo ParadoxOS Windows Native Build
echo ========================================
echo.

echo Setting up Windows-native build environment...

:: Create tools directory
if not exist "tools" mkdir tools
cd tools

:: Download pre-built cross-compiler if not exists
if not exist "x86_64-elf-gcc.exe" (
    echo Downloading pre-built cross-compiler...
    echo Please download x86_64-elf-tools from:
    echo https://github.com/lordmilko/i686-elf-tools/releases
    echo.
    echo Extract to: %cd%
    echo Then run this script again.
    pause
    exit /b 1
)

:: Add tools to PATH for this session
set PATH=%cd%;%PATH%
cd ..

:: Check tools
echo Checking tools...
tools\x86_64-elf-gcc.exe --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: x86_64-elf-gcc not working!
    pause
    exit /b 1
)
echo ✓ Cross-compiler ready

:: Download NASM if needed
if not exist "tools\nasm.exe" (
    echo Please download NASM from:
    echo https://www.nasm.us/pub/nasm/releasebuilds/
    echo Extract nasm.exe to tools\ directory
    pause
    exit /b 1
)
echo ✓ NASM ready

:: Clean previous build
echo Cleaning previous build...
if exist "paradox.elf" del paradox.elf
for /r %%i in (*.o) do del "%%i"

:: Build kernel
echo Building ParadoxOS kernel...
echo.

:: Compile C sources
echo Compiling C sources...
for %%f in (src\kernel\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o %%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\arch\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\arch\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\mm\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\mm\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\sched\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\sched\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\lib\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\lib\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\drivers\video\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\drivers\video\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\drivers\input\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\drivers\input\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\drivers\serial\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\drivers\serial\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\gui\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\gui\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\fs\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\fs\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

for %%f in (src\kernel\app\*.c) do (
    echo Compiling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\app\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

:: Compile assembly sources
echo Compiling assembly sources...
for %%f in (src\kernel\arch\*.S) do (
    echo Assembling %%f...
    tools\x86_64-elf-gcc.exe -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -Isrc/include -Ilimine -c %%f -o src\kernel\arch\%%~nf.o
    if %errorlevel% neq 0 goto :error
)

:: Link kernel
echo Linking kernel...
tools\x86_64-elf-ld.exe -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T linker.ld -o paradox.elf *.o src\kernel\arch\*.o src\kernel\mm\*.o src\kernel\sched\*.o src\kernel\lib\*.o src\kernel\drivers\video\*.o src\kernel\drivers\input\*.o src\kernel\drivers\serial\*.o src\kernel\gui\*.o src\kernel\fs\*.o src\kernel\app\*.o

if %errorlevel% neq 0 goto :error

echo.
echo ✓ Kernel built successfully: paradox.elf

:: Create ISO structure
echo Creating ISO structure...
if not exist "build" mkdir build
if not exist "build\iso" mkdir build\iso
if not exist "build\iso\boot" mkdir build\iso\boot

copy paradox.elf build\iso\boot\ >nul
copy limine.cfg build\iso\boot\ >nul

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Files created:
echo   📁 paradox.elf - Kernel binary
echo   📁 build\iso\ - ISO structure
echo.
echo Next steps:
echo 1. Download Limine bootloader files
echo 2. Create bootable ISO
echo 3. Test in VirtualBox
echo.
echo See BUILDING.md for complete instructions.
goto :end

:error
echo.
echo ❌ BUILD FAILED!
echo Check the error messages above.
pause
exit /b 1

:end
pause