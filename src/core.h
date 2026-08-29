#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t opcode;
  uint8_t memory[4096];
  uint8_t V[16];
  uint16_t I;
  uint16_t pc;

  uint8_t gfx[64 * 32];
  uint8_t delay_timer;
  uint8_t sound_timer;

  bool draw_flag;

  uint16_t stack[16];
  uint8_t sp;

  uint8_t key[16];
} Chip8;

enum {
  CHIP8_MEM_SIZE = 4096,
  CHIP8_FONT_ADDR = 0x050,
  CHIP8_ROM_START = 0x200,
  CHIP8_FONT_BYTES = 80,
};

void chip8_init(Chip8 *c);
bool chip8_load_rom(Chip8 *c, const char *path);
void chip8_cycle(Chip8 *c);
