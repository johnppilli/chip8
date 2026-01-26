# CHIP-8 Emulator

A CHIP-8 virtual machine written in C using SDL2 for graphics and input.

## Features

- Complete CPU emulation with 35 opcodes
- 64x32 monochrome display with sprite rendering
- Hex keypad input mapped to QWERTY
- 60Hz delay and sound timers

## Build

```bash
gcc -o chip8 src/main.c src/core.c -lSDL2
```

## Run

```bash
./chip8 roms/PONG.ch8
```

## Controls

```
CHIP-8      Keyboard
1 2 3 C     1 2 3 4
4 5 6 D     Q W E R
7 8 9 E     A S D F
A 0 B F     Z X C V
```
