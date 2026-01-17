# Building ParadoxOS

This guide explains how to build ParadoxOS from source and create a bootable ISO.

## Prerequisites

ParadoxOS requires specific tools to build properly:

### Required Tools
1. **x86_64 Cross-Compiler** - `x86_64-elf-gcc` and `x86_64-elf-ld`
2. **NASM Assembler** - For assembly code compilation
3. **Limine Bootloader** - Modern UEFI/BIOS bootloader
4. **xorriso** - For ISO creation
5. **QEMU** - For testing (optional)

## Setup Options

### Option 1: Windows with WSL (Recommended)

1. **Install WSL2 with Ubuntu:**
   ```bash
   wsl --install -d Ubuntu
   ```

2. **Install build tools in WSL:**
   ```bash
   sudo apt update
   sudo apt install build-essential nasm xorriso qemu-system-x86
   
   # Install cross-compiler
   sudo apt install gcc-multilib
   
   # Or build cross-compiler from source:
   # sudo apt install libgmp3-dev libmpc-dev libmpfr-dev texinfo
   # Then follow: https://wiki.osdev.org/GCC_Cross-Compiler
   ```

3. **Clone and build:**
   ```bash
   cd /mnt/c/path/to/paradoxos
   make clean
   make
   make iso
   ```

### Option 2: Docker Build (Easy)

1. **Create Dockerfile:**
   ```dockerfile
   FROM ubuntu:22.04
   
   RUN apt-get update && apt-get install -y \
       build-essential \
       nasm \
       xorriso \
       wget \
       git \
       && rm -rf /var/lib/apt/lists/*
   
   # Install cross-compiler (simplified - use pre-built)
   RUN apt-get update && apt-get install -y gcc-multilib
   
   WORKDIR /build
   ```

2. **Build with Docker:**
   ```bash
   docker build -t paradoxos-builder .
   docker run -v ${PWD}:/build paradoxos-builder make clean && make
   ```

### Option 3: Windows Native (Advanced)

1. **Download pre-built cross-compiler:**
   - Get from: https://github.com/lordmilko/i686-elf-tools
   - Or: https://github.com/sjitech/mingw-w64-x86_64-toolchain

2. **Install NASM:**
   - Download from: https://www.nasm.us/pub/nasm/releasebuilds/
   - Add to PATH

3. **Install xorriso:**
   - Part of cdrtools or get from: http://scdbackup.sourceforge.net/xorriso_eng.html

## Building Steps

### 1. Build the Kernel

```bash
# Clean previous build
make clean

# Build kernel
make

# This creates: paradox.elf
```

### 2. Setup Limine Bootloader

```bash
# Download Limine
wget https://github.com/limine-bootloader/limine/releases/download/v4.20231210.0/limine-4.20231210.0.tar.xz
tar -xf limine-4.20231210.0.tar.xz
cd limine-4.20231210.0
make
cd ..

# Copy required files
cp limine-4.20231210.0/limine-cd.bin limine/
cp limine-4.20231210.0/limine-cd-efi.bin limine/
cp limine-4.20231210.0/limine.sys limine/
```

### 3. Create Bootable ISO

```bash
# Create ISO structure
make iso-structure

# Create bootable ISO
xorriso -as mkisofs \
    -b boot/limine-cd.bin \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    --efi-boot boot/limine-cd-efi.bin \
    -efi-boot-part \
    --efi-boot-image \
    --protective-msdos-label \
    build/iso \
    -o paradox.iso

# Install Limine to ISO
./limine-4.20231210.0/limine-deploy paradox.iso
```

## Testing

### QEMU Testing
```bash
# Test kernel directly
qemu-system-x86_64 -kernel paradox.elf -m 256M

# Test ISO
qemu-system-x86_64 -cdrom paradox.iso -m 256M

# With graphics and networking
qemu-system-x86_64 -cdrom paradox.iso -m 256M -vga std -netdev user,id=net0 -device rtl8139,netdev=net0
```

### VirtualBox Testing
1. Create new VM (Type: Other, Version: Other/Unknown 64-bit)
2. Allocate 256MB+ RAM
3. Create virtual hard disk (optional)
4. Mount paradox.iso as CD/DVD
5. Boot from CD/DVD

### Real Hardware
⚠️ **Warning:** Only test on hardware you're willing to risk!

1. Burn ISO to USB/CD
2. Boot from USB/CD
3. ParadoxOS should start with desktop environment

## Troubleshooting

### Build Errors

**"x86_64-elf-gcc not found"**
- Install proper cross-compiler (see setup options above)

**"NASM not found"**
- Install NASM assembler

**"Linker errors"**
- Check that linker.ld is present and correct
- Verify all object files are being created

### Runtime Issues

**Black screen**
- Check QEMU graphics settings
- Try different VGA modes

**Kernel panic**
- Check serial output for debug information
- Verify memory allocation

**No mouse/keyboard**
- Ensure PS/2 emulation is enabled in VM

## Advanced Building

### Custom Configuration

Edit `src/include/paradox.h` to modify:
- Memory limits
- Maximum windows
- Debug levels

### Adding Features

1. Add source files to Makefile
2. Include headers in appropriate files
3. Initialize in `startup.c`
4. Test thoroughly

### Cross-Platform Notes

- **Linux:** Use native package manager for tools
- **macOS:** Use Homebrew: `brew install x86_64-elf-gcc nasm xorriso`
- **Windows:** WSL recommended for best compatibility

## File Structure After Build

```
paradox-os/
├── paradox.elf          # Kernel binary
├── paradox.iso          # Bootable ISO
├── build/
│   └── iso/             # ISO contents
│       ├── boot/
│       │   ├── paradox.elf
│       │   ├── limine.cfg
│       │   ├── limine-cd.bin
│       │   └── limine-cd-efi.bin
│       └── limine.sys
└── limine/              # Bootloader files
```

## Success Indicators

When ParadoxOS boots successfully, you should see:
1. Limine bootloader menu
2. Kernel initialization messages
3. Desktop environment with taskbar
4. Multiple application windows
5. Working mouse cursor
6. Keyboard input

The system is fully functional with:
- Window management
- File system
- Multiple applications
- Memory management
- Process scheduling

Enjoy your custom operating system! 🎉