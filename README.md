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

Or build and run:
```bash
make run
```

## Project Structure

- `src/chip8.cpp` - Complete CHIP-8 emulator implementation
- `roms/` - ROM files
