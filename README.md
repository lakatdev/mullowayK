# MullowayK Operating System

MullowayK is a hobby x86 operating system written from scratch in C. It features a custom desktop environment with built-in tools and includes an interpreter for the Keszeg 4 programming language.

## Features

### System Architecture
- **32-bit x86 Operating System** - Boots via GRUB multiboot
- **Fixed Memory Allocation** - Designed to consume exactly 500-512MB of RAM
- **Custom Kernel** - Written from scratch with minimal dependencies
- **Hardware Abstraction** - Direct hardware access and control

### Storage Support
- **IDE/ATA Drive Support** - Legacy IDE mode compatibility
- **SATA Drive Support** - Works with SATA drives in legacy IDE mode
- **PCI Bus Scanning** - Automatic detection of storage controllers
- **File System** - Custom storage system for file management to be changed later

### Hardware Features
- **Graphics Support** - Custom VGA graphics driver with framebuffer
- **Keyboard Input** - PS/2 keyboard driver with interrupt handling
- **Mouse Support** - PS/2 mouse driver for desktop interaction
- **Serial Port Communication** - COM1 serial port for I/O operations
- **Real-Time Clock** - System timing and date/time functionality
- **SSE Support** - SSE for enhanced performance

### Programming Environment
- **Keszeg 4 Language Interpreter** - Built-in interpreter for the Keszeg 4 custom programming language
- **Multiple Interpreter Instances** - Supports up to two more subordinate interpreter sessions

### Desktop Environment
- **Custom GUI** - Built-in desktop environment with simple window management
- **Built-in tools**:
  - **Text Editor** - Text and code editor mainly for the Keszeg code
  - **File Manager** - Browse and manage files on the storage device
  - **Runtime Environment** - Execute Keszeg 4 programs interactively
  - **System Information** - Display system details and version info
  - **Debug Tools** - Buffer for system wide debug messages

### Memory Management
- **No allocation** - No system-wide malloc and free for stability
- **Stack-based Function Calls** - Proper call stack management
- **Variable Scoping** - Local and reference parameter support
- **Garbage Collection** - Automatic memory cleanup for interpreter

## Building and Running

### Prerequisites
- GCC cross-compiler for x86 target
- GNU Make
- GRUB utilities for creating bootable ISO
- QEMU for emulation (optional)

### Build Instructions
```bash
# Build the operating system
make build

# Run in QEMU emulator
make run

# Clean build artifacts
make clean
```

### System Requirements
- **RAM**: Exactly 512MB (system is designed for this fixed amount)
- **Storage**: IDE or other drive in legacy IDE mode
- **Architecture**: x86 (32-bit)

## Architecture Details

### Kernel Components
- **Boot Loader** (`src/loader.s`) - Multiboot-compliant boot sequence
- **Global Descriptor Table** (`src/gdt.c`) - Memory segmentation setup
- **Interrupt Handling** (`src/interrupts.c`) - Hardware interrupt management
- **Memory Utils** (`src/memory.c`) - Well known memory functions
- **Graphics System** (`src/graphics.c`) - VGA framebuffer management

### Hardware Drivers
- **ATA/IDE Driver** (`src/ata.c`) - Storage device communication
- **Keyboard Driver** (`src/keyboard.c`) - PS/2 keyboard input
- **Mouse Driver** (`src/mouse.c`) - PS/2 mouse input
- **Serial Driver** (`src/serial.c`) - COM port communication
- **PCI Driver** (`src/pci.c`) - PCI bus enumeration

### Applications
- **Text Editor** (`src/apps/editor.c`) - Code editing with 16MB buffer
- **Runtime** (`src/apps/runtime.c`) - Keszeg 4 interpreter interface
- **File Manager** (`src/apps/files.c`) - File system browser
- **System Info** (`src/apps/info.c`) - System information display

### Keszeg 4 Interpreter
- **Parser** (`src/interpreter/interpreter.c`) - Code parsing and tokenization
- **Instruction Set** (`src/interpreter/instructions.c`) - Command implementation
- **Type System** (`src/interpreter/types.h`) - Variable type definitions
- **Function System** (`src/interpreter/function.h`) - Function struct

## Programming with Keszeg 4

The built-in Keszeg 4 interpreter supports:

- **Data Types**: `int`, `float`, `byte`, `string`, arrays
- **Control Structures**: `IF`/`ELSE`, loops, function calls
- **I/O Operations**: `PRINT`, `PRINTLN`, `INPUT`, file operations
- **File System**: `LOAD`, `SAVE` operations for persistent storage
- **Mathematical Operations**: Basic arithmetic and comparison operators
- **Memory Management**: Dynamic variable allocation and cleanup

Example Keszeg 4 program:
```
FUN main
    @ x int
    INPUT $ x
    IF x > 10
        PRINT const Hello\c World!\s
        PRINT const The answer is:
        PRINTLN $ x
    END
EF
```

## Version Information
- **Version**: 1.1.0
- **Build Date**: 2025-10-08
- **Interpreter**: [Keszeg 4](https://keszeglab.hu/keszeg4.html)
- **Target Architecture**: x86 (32-bit)

## Development
MullowayK is a personal hobby project demonstrating low-level system programming, custom operating system development, and language interpreter implementation. The system showcases direct hardware control, custom memory management, and a complete desktop computing environment built from the ground up. This project is a personal hobby operating system. Please respect the educational nature of this work.
