#!/bin/bash

echo "========================================="
echo "ParadoxOS Docker Build Script"
echo "========================================="
echo

# Check if we're in the right directory
if [ ! -f "src/kernel/main.c" ]; then
    echo "ERROR: Not in ParadoxOS source directory!"
    echo "Please run this from the root of the ParadoxOS project."
    exit 1
fi

echo "Setting up build environment..."

# Copy Limine files to project
echo "Setting up Limine bootloader..."
mkdir -p limine
cp /opt/limine-cd.bin limine/
cp /opt/limine-cd-efi.bin limine/
cp /opt/limine.sys limine/
echo "✓ Limine files copied"

# Clean previous build
echo "Cleaning previous build..."
make -f Makefile.cross clean

# Build the kernel
echo "Building ParadoxOS kernel..."
make -f Makefile.cross CC=x86_64-elf-gcc LD=x86_64-elf-ld

if [ $? -ne 0 ]; then
    echo "❌ Kernel build failed!"
    exit 1
fi

echo "✓ Kernel built successfully: paradox.elf"

# Create ISO structure
echo "Creating ISO structure..."
make -f Makefile.cross iso-structure

# Create bootable ISO
echo "Creating bootable ISO..."
make -f Makefile.cross iso

if [ -f "paradox.iso" ]; then
    echo "✓ Bootable ISO created: paradox.iso"
    echo
    echo "========================================="
    echo "BUILD SUCCESSFUL!"
    echo "========================================="
    echo
    echo "Files created:"
    echo "  - paradox.elf (kernel binary)"
    echo "  - paradox.iso (bootable ISO)"
    echo
    echo "To test in QEMU:"
    echo "  qemu-system-x86_64 -cdrom paradox.iso -m 256M"
    echo
    echo "To test kernel directly:"
    echo "  qemu-system-x86_64 -kernel paradox.elf -m 256M"
    echo
else
    echo "❌ ISO creation failed!"
    exit 1
fi