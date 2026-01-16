# Project Status

**Current Version**: 0.0.1 (Pre-Alpha)
**Current Phase**: Phase 1 (Kernel Bootstrap)

## ✅ Completed
-   [x] Project Directory Structure created.
-   [x] Initial Documentation (`README`, `ARCHITECTURE`, `ROADMAP`) written.
-   [x] **Strategic Analysis**: Analyzed Linux Kernel 2024-2025 documentation and integrated long-term goals (Rust, SMP, Security) into the project vision.
-   [x] Build scripts (`Makefile`, `build.bat`) implemented for Windows.
-   [x] **Kernel Bootstrap**:
    -   [x] Minimal C Kernel (`main.c`) written.
    -   [x] Kernel compiles and links successfully (`kernel.elf`).
-   [x] **Hardware Interactivity**:
    -   [x] GDT/IDT (CPU) initialized.
    -   [x] Keyboard & Mouse drivers active.
    -   [x] Pushed to GitHub [ParadoxOS](https://github.com/ethcocoder/paradoxos.git).

## 🚧 In Progress
-   [ ] **Phase 2.5 (Interactivity)**: Window Dragging and UI responsiveness.
-   [ ] **Bootable Media**: Creating ISO (Manual process verified, auto-process requires user setup).


## 🐛 Known Issues
-   `xorriso` is missing, preventing automatic ISO generation.
-   MinGW linker defaults to PE format (Workaround applied).
