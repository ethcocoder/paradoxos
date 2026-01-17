# ParadoxOS Setup Guide for Windows

## Quick Setup (Recommended)

### Option 1: WSL2 Setup (Easiest)

1. **Install WSL2:**
   ```powershell
   # Run in PowerShell as Administrator
   wsl --install -d Ubuntu
   ```

2. **Restart your computer** when prompted.

3. **Open Ubuntu WSL** and run these commands:
   ```bash
   # Update package list
   sudo apt update

   # Install build tools
   sudo apt install -y build-essential nasm xorriso qemu-system-x86 wget git

   # Install cross-compiler (simplified approach)
   sudo apt install -y gcc-multilib

   # Navigate to your project (adjust path as needed)
   cd /mnt/c/Users/fitsum.DESKTOP-JDUVJ6V/Downloads/paradoxos

   # Build ParadoxOS
   make -f Makefile.cross clean
   make -f Makefile.cross
   ```

### Option 2: Pre-built Cross-Compiler (Windows Native)

1. **Download pre-built toolchain:**
   - Go to: https://github.com/lordmilko/i686-elf-tools/releases
   - Download the latest x86_64-elf-tools release
   - Extract to `C:\cross-compiler\`

2. **Add to PATH:**
   - Add `C:\cross-compiler\bin` to your Windows PATH
   - Restart command prompt

3. **Install NASM:**
   - Download from: https://www.nasm.us/pub/nasm/releasebuilds/
   - Install and add to PATH

4. **Build:**
   ```cmd
   make -f Makefile.cross
   ```

## Building ParadoxOS

Once you have the cross-compiler set up:

```bash
# Clean any previous build
make -f Makefile.cross clean

# Build the kernel
make -f Makefile.cross

# Create ISO structure
make -f Makefile.cross iso-structure

# Download Limine bootloader (if not already done)
wget https://github.com/limine-bootloader/limine/releases/download/v4.20231210.0/limine-4.20231210.0.tar.xz
tar -xf limine-4.20231210.0.tar.xz
cd limine-4.20231210.0
make
cp limine-cd.bin limine-cd-efi.bin limine.sys ../limine/
cd ..

# Create bootable ISO
make -f Makefile.cross iso
```

## Testing

### Test in QEMU
```bash
# Test kernel directly
qemu-system-x86_64 -kernel paradox.elf -m 256M

# Test bootable ISO
qemu-system-x86_64 -cdrom paradox.iso -m 256M -vga std
```

### Test in VirtualBox
1. Create new VM (Other/Unknown 64-bit)
2. Allocate 256MB+ RAM
3. Mount paradox.iso as CD/DVD
4. Boot from CD/DVD

## Expected Results

When ParadoxOS boots successfully, you should see:

1. **Limine Bootloader** - Boot menu with ParadoxOS option
2. **Kernel Boot Messages** - Initialization progress
3. **Desktop Environment** - Full GUI with:
   - Desktop background
   - Taskbar at bottom
   - Start button
   - System clock
   - Mouse cursor

4. **Interactive Features:**
   - Click Start button to open applications
   - Text Editor, Calculator, File Manager, System Info
   - Window management (move, resize, close)
   - Keyboard and mouse input

## Troubleshooting

### "x86_64-elf-gcc not found"
- Install proper cross-compiler (see options above)
- Verify it's in your PATH: `which x86_64-elf-gcc`

### "NASM not found"
- Install NASM assembler
- Add to PATH

### "Build errors"
- Make sure you're using `Makefile.cross` not `Makefile`
- Check that all source files are present

### "Black screen in QEMU"
- Try: `qemu-system-x86_64 -cdrom paradox.iso -m 256M -vga std -serial stdio`
- Check for error messages in terminal

### "Kernel panic"
- Check serial output for debug information
- Verify memory allocation (try -m 512M)

## Success Indicators

✅ **Build Success:**
- `paradox.elf` file created (kernel binary)
- `paradox.iso` file created (bootable ISO)
- No compilation errors

✅ **Boot Success:**
- Limine bootloader appears
- Kernel initialization messages
- Desktop environment loads
- Mouse cursor visible and responsive

✅ **Full Functionality:**
- Applications can be launched
- Windows can be moved and closed
- Keyboard input works
- System appears stable

Your ParadoxOS is now ready for testing and further development! 🎉