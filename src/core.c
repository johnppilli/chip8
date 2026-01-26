#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t fontset[CHIP8_FONT_BYTES] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
};

void chip8_init(Chip8 *c) {
  memset(c, 0, sizeof(Chip8));
  c->pc = CHIP8_ROM_START;
  memcpy(&c->memory[CHIP8_FONT_ADDR], fontset, sizeof(fontset));
  c->draw_flag = true;
}

bool chip8_load_rom(Chip8 *c, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;

  size_t cap = sizeof(c->memory) - CHIP8_ROM_START;
  size_t n = fread(&c->memory[CHIP8_ROM_START], 1, cap, f);
  fclose(f);

  return n > 0;
}

void chip8_cycle(Chip8 *c) {
  // Fetch
  uint16_t op = (c->memory[c->pc] << 8) | c->memory[c->pc + 1];
  c->opcode = op;
  c->pc += 2;

  // Decode
  uint16_t nnn = op & 0x0FFF;
  uint8_t kk = op & 0x00FF;
  uint8_t n = op & 0x000F;
  uint8_t x = (op >> 8) & 0x0F;
  uint8_t y = (op >> 4) & 0x0F;

  // Execute
  switch (op & 0xF000) {
  case 0x0000:
    switch (op) {
    case 0x00E0:  // CLS - Clear display
      memset(c->gfx, 0, sizeof(c->gfx));
      c->draw_flag = true;
      break;

    case 0x00EE:  // RET - Return from subroutine
      if (c->sp == 0) {
        fprintf(stderr, "Stack underflow at PC=0x%03X\n", c->pc - 2);
        return;
      }
      c->pc = c->stack[--c->sp];
      break;

    default:
      break;
    }
    break;

  case 0x1000:  // JP addr - Jump to nnn
    c->pc = nnn;
    break;

  case 0x2000:  // CALL addr - Call subroutine at nnn
    if (c->sp >= 16) {
      fprintf(stderr, "Stack overflow at PC=0x%03X\n", c->pc - 2);
      return;
    }
    c->stack[c->sp++] = c->pc;
    c->pc = nnn;
    break;

  case 0x3000:  // SE Vx, byte - Skip if Vx == kk
    if (c->V[x] == kk)
      c->pc += 2;
    break;

  case 0x4000:  // SNE Vx, byte - Skip if Vx != kk
    if (c->V[x] != kk)
      c->pc += 2;
    break;

  case 0x5000:  // SE Vx, Vy - Skip if Vx == Vy
    if ((op & 0x000F) == 0 && c->V[x] == c->V[y])
      c->pc += 2;
    break;

  case 0x6000:  // LD Vx, byte - Set Vx = kk
    c->V[x] = kk;
    break;

  case 0x7000:  // ADD Vx, byte - Set Vx = Vx + kk
    c->V[x] += kk;
    break;

  case 0x8000:
    switch (op & 0x000F) {
    case 0x0:  // LD Vx, Vy
      c->V[x] = c->V[y];
      break;
    case 0x1:  // OR Vx, Vy
      c->V[x] |= c->V[y];
      break;
    case 0x2:  // AND Vx, Vy
      c->V[x] &= c->V[y];
      break;
    case 0x3:  // XOR Vx, Vy
      c->V[x] ^= c->V[y];
      break;
    case 0x4: {  // ADD Vx, Vy (with carry)
      uint16_t sum = c->V[x] + c->V[y];
      c->V[0xF] = sum > 0xFF;
      c->V[x] = sum & 0xFF;
      break;
    }
    case 0x5:  // SUB Vx, Vy
      c->V[0xF] = c->V[x] >= c->V[y];
      c->V[x] -= c->V[y];
      break;
    case 0x6:  // SHR Vx
      c->V[0xF] = c->V[x] & 0x1;
      c->V[x] >>= 1;
      break;
    case 0x7:  // SUBN Vx, Vy
      c->V[0xF] = c->V[y] >= c->V[x];
      c->V[x] = c->V[y] - c->V[x];
      break;
    case 0xE:  // SHL Vx
      c->V[0xF] = (c->V[x] >> 7) & 0x1;
      c->V[x] <<= 1;
      break;
    }
    break;

  case 0x9000:  // SNE Vx, Vy - Skip if Vx != Vy
    if ((op & 0x000F) == 0 && c->V[x] != c->V[y])
      c->pc += 2;
    break;

  case 0xA000:  // LD I, addr - Set I = nnn
    c->I = nnn;
    break;

  case 0xB000:  // JP V0, addr - Jump to V0 + nnn
    c->pc = c->V[0] + nnn;
    break;

  case 0xC000:  // RND Vx, byte - Set Vx = random & kk
    c->V[x] = (rand() % 256) & kk;
    break;

  case 0xD000: {  // DRW Vx, Vy, n - Draw sprite
    uint8_t vx = c->V[x] % 64;
    uint8_t vy = c->V[y] % 32;
    c->V[0xF] = 0;

    for (uint8_t row = 0; row < n; row++) {
      uint8_t sprite_byte = c->memory[c->I + row];
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (sprite_byte & (0x80 >> bit)) {
          uint8_t px = (vx + bit) % 64;
          uint8_t py = (vy + row) % 32;
          uint16_t idx = py * 64 + px;

          if (c->gfx[idx])
            c->V[0xF] = 1;
          c->gfx[idx] ^= 1;
        }
      }
    }
    c->draw_flag = true;
    break;
  }

  case 0xE000: {
    uint8_t key = c->V[x] & 0x0F;
    switch (op & 0x00FF) {
    case 0x9E:  // SKP Vx - Skip if key pressed
      if (c->key[key])
        c->pc += 2;
      break;
    case 0xA1:  // SKNP Vx - Skip if key not pressed
      if (!c->key[key])
        c->pc += 2;
      break;
    }
    break;
  }

  case 0xF000:
    switch (op & 0x00FF) {
    case 0x07:  // LD Vx, DT
      c->V[x] = c->delay_timer;
      break;

    case 0x0A: {  // LD Vx, K - Wait for key press
      int pressed = -1;
      for (int k = 0; k < 16; k++) {
        if (c->key[k]) {
          pressed = k;
          break;
        }
      }
      if (pressed < 0) {
        c->pc -= 2;  // Re-execute this instruction
        return;
      }
      c->V[x] = pressed;
      break;
    }

    case 0x15:  // LD DT, Vx
      c->delay_timer = c->V[x];
      break;

    case 0x18:  // LD ST, Vx
      c->sound_timer = c->V[x];
      break;

    case 0x1E:  // ADD I, Vx
      c->I += c->V[x];
      break;

    case 0x29:  // LD F, Vx - Set I to font sprite location
      c->I = CHIP8_FONT_ADDR + (c->V[x] & 0x0F) * 5;
      break;

    case 0x33:  // LD B, Vx - Store BCD of Vx
      if (c->I + 2 >= CHIP8_MEM_SIZE) {
        fprintf(stderr, "I out of bounds (0x%03X) at PC=0x%03X\n", c->I, c->pc - 2);
        return;
      }
      c->memory[c->I] = c->V[x] / 100;
      c->memory[c->I + 1] = (c->V[x] / 10) % 10;
      c->memory[c->I + 2] = c->V[x] % 10;
      break;

    case 0x55:  // LD [I], Vx - Store V0-Vx to memory
      if (c->I + x >= CHIP8_MEM_SIZE) {
        fprintf(stderr, "I out of bounds (0x%03X) at PC=0x%03X\n", c->I, c->pc - 2);
        return;
      }
      for (uint8_t i = 0; i <= x; i++)
        c->memory[c->I + i] = c->V[i];
      c->I += x + 1;
      break;

    case 0x65:  // LD Vx, [I] - Load V0-Vx from memory
      if (c->I + x >= CHIP8_MEM_SIZE) {
        fprintf(stderr, "I out of bounds (0x%03X) at PC=0x%03X\n", c->I, c->pc - 2);
        return;
      }
      for (uint8_t i = 0; i <= x; i++)
        c->V[i] = c->memory[c->I + i];
      c->I += x + 1;
      break;
    }
    break;
  }
}
