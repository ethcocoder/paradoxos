# Project Status

**Current Version**: 0.0.1 (Pre-Alpha)
**Current Phase**: Phase 1 (Kernel Bootstrap)

## ✅ Completed
-   [x] Project Directory Structure created.
-   [x] Initial Documentation (`README`, `ARCHITECTURE`) written.
-   [x] Build scripts (`Makefile`, `build.bat`) implemented for Windows.
-   [x] **Kernel Bootstrap**:
    -   [x] Minimal C Kernel (`main.c`) written.
    -   [x] Kernel compiles and links successfully (`kernel.elf`).

## 🚧 In Progress
-   [ ] **Bootable Media**:
    -   [ ] Creating ISO or Disk Image (Blocked by missing `xorriso`).
    -   [ ] Testing in VMware.

## 🐛 Known Issues
-   `xorriso` is missing, preventing automatic ISO generation.
-   MinGW linker defaults to PE format (Workaround applied).
