#!/bin/bash

echo "========================================="
echo "ParadoxOS WSL Build Script"
echo "========================================="
echo

# Check if we're in WSL
if [[ ! -f /proc/version ]] || ! grep -q Microsoft /proc/version; then
    echo "This script should be run in WSL (Windows Subsystem for Linux)"
    echo "Please run: wsl"
    echo "Then run this script again"
    exit 1
fi

# Update package list
echo "Updating package list..."
sudo apt update

# Install required packages
echo "Installing build tools..."
sudo apt install -y build-essential nasm xorriso qemu-system-x86 wget git curl

# Install cross-compiler (using multilib approach for simplicity)
echo "Installing cross-compiler..."
sudo apt install -y gcc-multilib

# Verify tools are installed
echo
echo "Verifying installation..."
echo -n "GCC: "
gcc --version | head -1
echo -n "NASM: "
nasm -version
echo -n "xorriso: "
xorriso --version | head -1
echo -n "QEMU: "
qemu-system-x86_64 --version | head -1

echo
echo "Setting up Limine bootloader..."

# Download and build Limine if not already present
if [ ! -f "limine/limine-cd.bin" ]; then
    echo "Downloading Limine bootloader..."
    wget -q https://github.com/limine-bootloader/limine/releases/download/v4.20231210.0/limine-4.20231210.0.tar.xz
    
    echo "Extracting and building Limine..."
    tar -xf limine-4.20231210.0.tar.xz
    cd limine-4.20231210.0
    make -j$(nproc)
    
    # Copy files to project
    mkdir -p ../limine
    cp limine-cd.bin limine-cd-efi.bin limine.sys ../limine/
    cd ..
    
    # Clean up
    rm -rf limine-4.20231210.0*
    echo "✓ Limine bootloader ready"
else
    echo "✓ Limine bootloader already present"
fi

echo
echo "Building ParadoxOS..."

# Clean previous build
echo "Cleaning previous build..."
make -f Makefile.cross clean

# Build kernel
echo "Building kernel..."
make -f Makefile.cross CC=gcc LD=ld

if [ $? -ne 0 ]; then
    echo "❌ Kernel build failed!"
    echo "Check the error messages above."
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
    
    # Get file sizes
    KERNEL_SIZE=$(ls -lh paradox.elf | awk '{print $5}')
    ISO_SIZE=$(ls -lh paradox.iso | awk '{print $5}')
    
    echo
    echo "========================================="
    echo "🎉 BUILD SUCCESSFUL! 🎉"
    echo "========================================="
    echo
    echo "Files created:"
    echo "  📁 paradox.elf ($KERNEL_SIZE) - Kernel binary"
    echo "  💿 paradox.iso ($ISO_SIZE) - Bootable ISO"
    echo
    echo "Testing options:"
    echo "  🖥️  Test kernel: qemu-system-x86_64 -kernel paradox.elf -m 256M"
    echo "  💿 Test ISO:    qemu-system-x86_64 -cdrom paradox.iso -m 256M -vga std"
    echo
    echo "Your custom operating system is ready! 🚀"
    echo
    
    # Ask if user wants to test immediately
    read -p "Would you like to test ParadoxOS in QEMU now? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Starting ParadoxOS in QEMU..."
        echo "Press Ctrl+Alt+G to release mouse, Ctrl+Alt+Q to quit"
        qemu-system-x86_64 -cdrom paradox.iso -m 256M -vga std -serial stdio
    fi
else
    echo "❌ ISO creation failed!"
    echo "Check the error messages above."
    exit 1
fi