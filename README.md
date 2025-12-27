# CHIP-8 Emulator

A single-file CHIP-8 emulator implementation in C++ with SDL2.

## Prerequisites

- macOS with Xcode Command Line Tools
- SDL2 library

## Setup

Install SDL2:
```bash
brew install sdl2
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
