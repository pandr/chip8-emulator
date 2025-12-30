# CHIP-8 Emulator

A single-file CHIP-8 emulator implementation in C++ with SDL2.

## Platform Support

This emulator runs on:
- macOS (Intel and ARM)
- Linux

## Prerequisites

### macOS
- Xcode Command Line Tools
- SDL2 library (via Homebrew)

### Linux
- GCC compiler
- SDL2 development libraries

## Setup

### macOS
Install SDL2:
```bash
brew install sdl2
```

### Linux
Install SDL2 (Debian/Ubuntu):
```bash
sudo apt-get install libsdl2-dev
```

## Building

Build the emulator:
```bash
make
```

## Running

Run the emulator with a ROM:
```bash
./bin/chip8 roms/yourrom.ch8
```

### Command Line Arguments

- `-s` - Enable stepping mode (press SPACE to step through instructions)
- `-t` - Show timing information (prints stats every 60 frames)
- `-c <num>` - Set cycles per frame (default: 8)

Example:
```bash
# Run with 12 cycles per frame
./bin/chip8 -c 12 roms/BRIX
```

## Project Structure

- `src/chip8.cpp` - Complete CHIP-8 emulator implementation
- `roms/` - ROM files
