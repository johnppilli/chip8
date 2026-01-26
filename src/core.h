#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t opcode;        // Current instruction
  uint8_t memory[4096];   // 4 KB RAM
  uint8_t V[16];          // General-purpose registers (V0-VF)
  uint16_t I;             // Index register
  uint16_t pc;            // Program counter

  uint8_t gfx[64 * 32];   // Framebuffer (1 bit per pixel)
  uint8_t delay_timer;
  uint8_t sound_timer;

  bool draw_flag;         // Set when display needs redraw

  uint16_t stack[16];     // Call stack for subroutines
  uint8_t sp;             // Stack pointer

  uint8_t key[16];        // Hex keypad state (0x0-0xF)
} Chip8;

enum {
  CHIP8_MEM_SIZE = 4096,
  CHIP8_FONT_ADDR = 0x050,
  CHIP8_ROM_START = 0x200,
  CHIP8_FONT_BYTES = 80,  // 16 digits * 5 bytes each
};

void chip8_init(Chip8 *c);
bool chip8_load_rom(Chip8 *c, const char *path);
void chip8_cycle(Chip8 *c);
