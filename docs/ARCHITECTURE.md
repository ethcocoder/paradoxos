# ParadoxOS Architecture

## Overview
ParadoxOS is designed as a hybrid-microkernel architecture, prioritizing isolation and stability without sacrificing the performance of a monolithic kernel.

## Layer 0: The Paradox Kernel
The heart of the system.
-   **Language**: C / C++ (with Assembly stubs).
-   **Responsibilities**:
    -   Bootstrapping (Multiboot2/Stivale2).
    -   Physical & Virtual Memory Management (PMM/VMM).
    -   Task Scheduling (Preemptive Multitasking).
    -   Interrupt Handling (IDT/GDT).

## Layer 1: System Executive
Manages system resources and security.
-   **Object Manager**: Unified handle system for resources.
-   **Security Monitor**: Capability-based access control.
-   **I/O Manager**: Asynchronous I/O processing.

## Layer 2: Driver Framework
A modular driver system heavily inspired by the Windows Driver Model (WDM) but simplified.
-   Drivers run in user-space (mostly) for stability.
-   Crash recovery: If a graphics driver crashes, the kernel restarts it without Blue Screening.

## Layer 4: The Desktop Environment (UI/UX)
**Critical Requirement**: The UI must be visually stunning and "Impressive".
-   **Compositor**: Custom window compositor supporting transparency, blur, and hardware acceleration.
-   **Theming Engine**: Deep system-wide theming support.
    -   *Themes*: Void, Aurora, Solar, Neo-Glass.
-   **animations**: "Living" animations that react to system load and user interaction.
