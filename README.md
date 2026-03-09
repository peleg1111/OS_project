# README - MyOS 32-bit Kernel

MyOS is a custom 32-bit x86 operating system kernel developed in C and Assembly. It operates in Protected Mode and interacts directly with hardware components through low-level drivers.

## Current Features

### 1. Display Management (VGA Driver)

* **VGA Text Mode**: Operates in the standard $80 \times 25$ characters mode using the memory address `0xB8000`.
* **Hardware Cursor Control**: Direct communication with the CRT Controller via ports `0x3D4` (Index) and `0x3D5` (Data) to update the visual cursor position.
* **Terminal Buffering**: Implements a `terminal_buffer` that stores character and attribute data (color) for the entire addressable screen area.
* **Vertical Scrolling**: Logic to shift the current view based on `current_view_row`, allowing the display to handle text output exceeding the physical 25-line limit.
* **Color Support**: Support for background and foreground color attributes for every character.

### 2. Input & Peripherals

* **Keyboard Driver**: Processes hardware interrupts from the PS/2 controller. It maps scan codes to ASCII characters.
* **Port I/O**: Custom implementations of `port_byte_in` and `port_byte_out` using inline Assembly (`inb`/`outb`) for hardware communication.
* **Special Key Support**:
* **Backspace**: Deletes characters from the buffer and moves the cursor back, with logic to protect the command prompt (marked by the `!` character).
* **Newline (`\n`)**: Resets the cursor to the start of the next line.
* **Tab (`\t`)**: Inserts four spaces for indentation.

### 3. Kernel Standard Library

* **Formatted Printing (`printf`)**: A custom implementation supporting multiple format specifiers:
* `%s`: Strings (rendered in green).
* `%c`: Characters (rendered in yellow).
* `%d`: Signed integers (rendered in red).
* `%f`: Floating-point numbers with adjustable precision.
* `%p`: Hexadecimal memory addresses.


* **Mathematical Utilities**: Basic integer-to-string conversion and float-to-string algorithms.

### 4. String Manipulation

* **In-place Splitting**: A `split` function that modifies the original string by inserting null terminators (`\0`) and populating an array of pointers to the resulting tokens.
* **String Utilities**: Custom versions of `strlen`, `strcmp`, and `replace`.

## Technical Specifications

* **Architecture**: x86 (Intel/AMD).
* **Mode**: 32-bit Protected Mode.
* **Memory Map**:
* `VIDEO_ADDRESS`: `0xB8000`.
* `MAX_ROWS`: Defined in `const.h` for the scrollback buffer size.

## Build Requirements

* **Compiler**: GCC (cross-compiler for `i386-elf` recommended).
* **Assembler**: NASM (for the bootloader and kernel entry).
* **Linker**: ld.
* **Emulator**: QEMU (for testing).