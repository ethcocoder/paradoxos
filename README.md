# ParadoxOS
> **"A Next-Generation Intelligent Operating System"**

ParadoxOS is a general-purpose operating system designed to merge **Windows-like usability** with **artistic identity** and **system intelligence**. It is not just a tool; it is an experience.

## 🌌 Core Philosophy
1.  **Impressive UI (The Soul)**: The interface is not static. It breathes. It features living animations, smooth transitions, and deep customization themes (Void, Aurora, Solar).
2.  **Intelligent Behavior (The Brain)**: The OS understands intent. It optimizes performance dynamically based on your workflow (e.g., "Coding Mode" vs "Battery Saver").
3.  **Familiarity**: Detailed to feel like home for Windows users, but faster, lighter, and more secure.

## 🏗️ Architecture
ParadoxOS is built on a 5-layer architecture:
-   **Layer 0: Kernel** - Lightweight, high-performance C/C++ kernel.
-   **Layer 1: System Executive** - Process/Memory management.
-   **Layer 2: Driver Framework** - Isolated, plug-and-play drivers.
-   **Layer 3: Application Runtime** - Secure containers for apps.
-   **Layer 4: Desktop Environment** - The visual masterpiece.

## 🛠️ Build & Run
### Requirements
-   `gcc` (Cross-compiler recommended)
-   `make`
-   `xorriso` (for ISO generation)
-   `qemu` or VMware Workstation

### Instructions
1.  **Build ISO**:
    ```bash
    make iso
    ```
2.  **Run in QEMU**:
    ```bash
    make run
    ```
3.  **Run in VMware**:
    -   Open VMware Workstation.
    -   Create a new VM.
    -   Select `build/paradoxos.iso` as the installer disc image.

## 📅 Roadmap
-   **Phase 1**: Kernel Bootstrap & Bootloader (Current)
-   **Phase 2**: Basic Desktop & Graphics
-   **Phase 3**: Hardware Drivers
-   **Phase 4**: Intelligent Optimization
-   **Phase 5**: Complete Visual Identity & Theming

---
*Powered by the ethco coders Team*
