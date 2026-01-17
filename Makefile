# ParadoxOS Makefile for Windows

# Toolchain definitions
CC = gcc
LD = ld
NASM = nasm

# Flags
CFLAGS = -g -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -Isrc/include -Ilimine
LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T linker.ld
NASMFLAGS = -f elf64

# Files (Windows compatible)
C_SOURCES = src/kernel/main.c src/kernel/startup.c src/kernel/arch/gdt.c src/kernel/arch/idt.c src/kernel/mm/pmm.c src/kernel/mm/vmm.c src/kernel/sched/scheduler.c src/kernel/lib/string.c src/kernel/lib/printk.c src/kernel/drivers/video/framebuffer.c src/kernel/drivers/input/keyboard.c src/kernel/drivers/serial/serial.c src/kernel/syscall.c src/kernel/gui/window.c src/kernel/fs/vfs.c src/kernel/app/framework.c src/kernel/system_status.c
ASM_SOURCES = src/kernel/arch/entry.S src/kernel/arch/interrupts.S src/kernel/arch/syscall.S
C_OBJ = $(C_SOURCES:.c=.o)
ASM_OBJ = $(ASM_SOURCES:.S=.o)
OBJ = $(C_OBJ) $(ASM_OBJ)

# Targets
all: paradox.elf

paradox.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# ISO creation (requires proper setup)
iso: paradox.elf setup-limine
	@echo "Creating ParadoxOS ISO..."
	@if not exist "build" mkdir build
	@if not exist "build\iso" mkdir build\iso
	@if not exist "build\iso\boot" mkdir build\iso\boot
	copy paradox.elf build\iso\boot\
	copy limine.cfg build\iso\boot\
	@echo "ISO directory structure created in build\iso\"
	@echo "To complete ISO creation, you need:"
	@echo "1. Download Limine bootloader files"
	@echo "2. Install xorriso for ISO creation"
	@echo "3. Run: xorriso -as mkisofs -b boot/limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine-cd-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label build/iso -o paradox.iso"

setup-limine:
	@echo "Setting up Limine bootloader..."
	@if not exist "limine\limine-cd.bin" (
		@echo "Limine bootloader files not found!"
		@echo "Please download Limine from: https://github.com/limine-bootloader/limine"
		@echo "And place the following files in the limine/ directory:"
		@echo "  - limine-cd.bin"
		@echo "  - limine-cd-efi.bin" 
		@echo "  - limine.sys"
	)

# Simple build test (without full cross-compiler)
build-test:
	@echo "Testing build with available tools..."
	@echo "Checking for required tools..."
	@where gcc >nul 2>&1 && echo "✓ GCC found" || echo "✗ GCC not found"
	@where ld >nul 2>&1 && echo "✓ LD found" || echo "✗ LD not found"
	@where nasm >nul 2>&1 && echo "✓ NASM found" || echo "✗ NASM not found"

clean:
	del /Q src\kernel\*.o 2>nul || echo.
	del /Q src\kernel\arch\*.o 2>nul || echo.
	del /Q src\kernel\mm\*.o 2>nul || echo.
	del /Q src\kernel\sched\*.o 2>nul || echo.
	del /Q src\kernel\lib\*.o 2>nul || echo.
	del /Q src\kernel\drivers\video\*.o 2>nul || echo.
	del /Q src\kernel\drivers\input\*.o 2>nul || echo.
	del /Q src\kernel\drivers\serial\*.o 2>nul || echo.
	del /Q src\kernel\gui\*.o 2>nul || echo.
	del /Q src\kernel\fs\*.o 2>nul || echo.
	del /Q src\kernel\app\*.o 2>nul || echo.
	del /Q paradox.elf 2>nul || echo.
	rmdir /S /Q build 2>nul || echo.

help:
	@echo "ParadoxOS Build System"
	@echo "====================="
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build the kernel (paradox.elf)"
	@echo "  iso          - Create bootable ISO (requires setup)"
	@echo "  build-test   - Test if build tools are available"
	@echo "  setup-limine - Check/setup Limine bootloader"
	@echo "  clean        - Clean build files"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Requirements for full build:"
	@echo "  - x86_64 cross-compiler (x86_64-elf-gcc)"
	@echo "  - NASM assembler"
	@echo "  - Limine bootloader files"
	@echo "  - xorriso (for ISO creation)"
