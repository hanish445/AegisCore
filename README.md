# AegisCore (v1.2.5): Dynamic Taint Analysis Engine & Binary Fuzzing Sandbox (Primarily also a GameBoy emulator)

## Overview

AegisCore is a custom, clean-room hardware architecture emulator of the Nintendo Game Boy (DMG-01) built in C20. While it functions as a highly accurate cycle-stepped emulator, its ultimate objective is to serve as an isolated runtime sandbox. The engine is designed to track untrusted data propagation, identify control-flow integrity violations, and automatically discover memory corruption vulnerabilities or malformed input behaviors in binary applications.

## Current State of the Project

The core hardware emulation is highly stable. The Sharp LR35902 CPU instruction interpreter, the Picture Processing Unit (PPU), and the Memory Management Unit (MMU) are fully operational. The MMU successfully implements memory mapping and Memory Bank Controller (MBC1) support, allowing for the execution of complex, multi-bank commercial binaries. The main execution loop has been optimized with frame-batching and precise chrono-based timing to achieve a stable 60 FPS output.

### Test Binaries

To ensure cycle accuracy and correct hardware behavior, **Tetris** (ROM-only) and **Pokemon Red** (MBC1, 1MB) were utilized as the primary test suites. Both titles currently boot, render correctly, and are fully playable within the emulator. 

*Note: Due to strict copyright laws, no ROM files (including Tetris and Pokemon Red) are committed to this repository. Users must supply their own legally dumped binaries to utilize the emulator.*

## Project Roadmap

The development of AegisCore is divided into six distinct phases. The foundational emulation is complete, and active development is shifting toward the security analysis and fuzzing components.

### Completed Phases

*   **Phase 1: Initial Setup**
    Establishment of the C++ project structure, build systems (Makefile), and foundational memory arrays for the 64KB 16-bit address space.
*   **Phase 2: Core Functionality**
    Implementation of the CPU instruction set, registers, ALU operations, and the baseline PPU for rendering backgrounds and sprites to the 160x144 framebuffer.
*   **Phase 3: Adjustments for Playability**
    Refinement of the emulator for actual usage, including Joypad input polling via the Win32 API, MBC1 cartridge banking, DIV/TIMA hardware timers, and frame-batching for smooth 60 FPS performance.
*   **Phase 4: Fuzzing & Logging (Simultaneous Python + JS)**
    Construction of external tooling to stress-test the emulator. A Python mutator will bombard the C++ engine with corrupted ROM data (bit-flips, magic-number injections). Upon crashing, the C++ engine will export diagnostic logs (trace.json), which a Node.js server will parse and display on a local web dashboard.

### Upcoming Phases
*   **Phase 5: Audio Processing Unit (APU) Emulation**
    Implementation of the DMG-01 audio hardware (Square, Wave, and Noise channels). This ensures complete emulation compatibility and opens a new surface area for the fuzzer to target audio registers (0xFF10 - 0xFF3F).
*   **Phase 6: Dynamic Taint Analysis (DTA) Module**
    The integration of a shadow memory architecture parallel to the MMU. This module will tag untrusted inputs and track their propagation through CPU registers and memory. Runtime hooks will enforce security policies, catching potential control-flow hijacks if tainted data attempts to modify the Program Counter (PC) or Stack Pointer (SP).

## Build Instructions

Currently, AegisCore is configured for Windows environments using MinGW/GCC.

1. Ensure `g++` (supporting C++20) and `make` are installed.
2. Clone the repository.
3. Run `make` in the root directory to build `build/AegisCore.exe`.
4. Run the executable via command line, passing the path to your ROM as an argument, or drag and drop a valid `.gb` file onto the compiled executable.
5. command: `\build\AegisCore.exe fuzzer\corpus\"your_rom.gb"` (in main.cpp one of my ROMs is hardcoded, make sure you replace it with yours. A feature to avoid this hard coding is underway...)
