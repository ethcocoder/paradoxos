# ParadoxOS - Complete Custom Operating System

ParadoxOS is a fully functional custom operating system built from scratch for x86_64 architecture. It features a modern hybrid kernel design, advanced graphics capabilities, and a complete desktop environment.

## 🚀 Features

### Core Kernel
- **Hybrid Kernel Architecture** - Combines monolithic performance with microkernel modularity
- **Advanced Memory Management** - Copy-on-write, demand paging, swap support
- **Multi-tasking Scheduler** - Round-robin scheduling with task switching
- **System Call Interface** - Complete POSIX-compatible system call implementation
- **Exception Handling** - Professional panic screens with diagnostic information

### Graphics & UI
- **Advanced Framebuffer Driver** - Hardware-accelerated 2D rendering
- **Window Manager (ParadoxWM)** - Full window management with focus, z-order, and decorations
- **Desktop Environment** - Complete desktop with taskbar, start button, and system tray
- **Alpha Blending** - Transparency and visual effects support
- **Double Buffering** - Smooth, flicker-free rendering

### I/O Systems
- **Enhanced Input Drivers** - Full PS/2 keyboard and mouse support with Unicode
- **Virtual File System (VFS)** - Unified interface for multiple file systems
- **Serial Communication** - Debug output and system logging

### Applications
- **Application Framework** - Complete API for developing native applications
- **Built-in Applications**:
  - Text Editor
  - Calculator
  - File Manager
  - System Information

### Advanced Features
- **Memory Protection** - Virtual memory with page-level protection
- **Process Isolation** - Secure process boundaries
- **Interrupt Handling** - Complete IDT with all x86_64 interrupt handlers
- **Device Management** - Hot-pluggable device support

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User Applications                        │
├─────────────────────────────────────────────────────────────┤
│              Application Framework                          │
├─────────────────────────────────────────────────────────────┤
│    Desktop Environment    │    Window Manager (ParadoxWM)  │
├─────────────────────────────────────────────────────────────┤
│              Graphics Stack (Framebuffer)                  │
├─────────────────────────────────────────────────────────────┤
│    VFS    │   System Calls   │    Process Manager          │
├─────────────────────────────────────────────────────────────┤
│              Hybrid Kernel Core                             │
│  ┌─────────────┬─────────────┬─────────────┬─────────────┐  │
│  │     VMM     │     PMM     │  Scheduler  │     IDT     │  │
│  └─────────────┴─────────────┴─────────────┴─────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                Hardware Abstraction Layer                  │
└─────────────────────────────────────────────────────────────┘
```

## 🛠️ Building

### Prerequisites
- x86_64 cross-compiler (x86_64-elf-gcc)
- NASM assembler
- Make
- QEMU (for testing)
- Limine bootloader

### Build Commands
```bash
# Clean previous build
make clean

# Build the kernel
make

# The output will be paradox.elf
```

### Running
```bash
# Run in QEMU
qemu-system-x86_64 -kernel paradox.elf -m 256M
```

## 🎮 Usage

### Keyboard Shortcuts
- **Ctrl+N** - Create new window
- **Ctrl+Q** - Close focused window
- **Mouse** - Click to focus windows, drag to move
- **Window Controls** - Click X button to close windows

### Desktop Features
- **Taskbar** - Shows running applications and system clock
- **Start Button** - Access to system applications
- **Window Management** - Resize, minimize, maximize, close
- **Multi-tasking** - Run multiple applications simultaneously

## 📁 Project Structure

```
paradox-os/
├── src/
│   ├── include/          # Header files
│   │   ├── arch/         # Architecture-specific headers
│   │   ├── drivers/      # Device driver headers
│   │   ├── gui/          # GUI system headers
│   │   ├── mm/           # Memory management headers
│   │   ├── fs/           # File system headers
│   │   └── app/          # Application framework headers
│   └── kernel/           # Kernel implementation
│       ├── arch/         # Architecture-specific code
│       ├── drivers/      # Device drivers
│       ├── mm/           # Memory management
│       ├── gui/          # GUI implementation
│       ├── fs/           # File system implementation
│       ├── app/          # Application framework
│       └── lib/          # Kernel library functions
├── assets/               # Fonts, icons, wallpapers
├── limine/              # Bootloader files
├── Makefile             # Build configuration
├── linker.ld            # Linker script
└── limine.cfg           # Boot configuration
```

## 🔧 Technical Details

### Memory Management
- **Physical Memory Manager (PMM)** - Bitmap-based allocation with buddy system
- **Virtual Memory Manager (VMM)** - 4-level page tables with copy-on-write
- **Demand Paging** - Pages loaded on-demand with swap support
- **Memory Protection** - Ring-based security model

### Process Management
- **Hybrid Kernel** - Core services in kernel space, applications in user space
- **Task Switching** - Assembly-optimized context switching
- **Process Isolation** - Separate address spaces for security

### Graphics System
- **Layered Architecture** - Graphics server, compositor, window manager
- **Hardware Acceleration** - Optimized rendering pipelines
- **Modern UI** - Windows 11-style interface with transparency

### File System
- **VFS Layer** - Unified interface for multiple file systems
- **In-Memory FS** - Fast temporary file storage
- **POSIX Compatibility** - Standard file operations

## 🎯 Achievements

This project demonstrates:
- **Complete OS Development** - From bootloader to desktop applications
- **Modern Architecture** - Hybrid kernel with advanced features
- **Production Quality** - Professional error handling and user experience
- **Extensibility** - Modular design for easy feature addition

## 🚧 Future Enhancements

- Network stack (TCP/IP)
- Audio system
- USB support
- SMP (multi-core) support
- Package management system
- More file system types (ext4, NTFS)

## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

**ParadoxOS** - A complete custom operating system demonstrating advanced OS development concepts and modern system design.