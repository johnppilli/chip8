#include "core.h"
#include <stdio.h>  // for printf (debugging)
#include <stdlib.h> // for rand()
#include <string.h> // for memset

void chip8_init(Chip8 *c) {
  memset(c, 0, sizeof(Chip8)); // clear everything in the struct
  c->pc = 0x200;               // Program counter starts at 0x200

  static const uint8_t fontset[chip8_font_bytes] = {
      0xF0, 0x90, 0x90, 0x90,
      0xF0, // 0
      0x20, 0x60, 0x20, 0x20,
      0x70, // 1
      0xF0, 0x10, 0xF0, 0x80,
      0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10,
      0xF0, // 3
      0x90, 0x90, 0xF0, 0x10,
      0x10, // 4
      0xF0, 0x80, 0xF0, 0x10,
      0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90,
      0xF0, // 6
      0xF0, 0x10, 0x20, 0x40,
      0x40, // 7
      0xF0, 0x90, 0xF0, 0x90,
      0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10,
      0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90,
      0x90, // A
      0xE0, 0x90, 0xE0, 0x90,
      0xE0, // B
      0xF0, 0x80, 0x80, 0x80,
      0xF0, // C
      0xE0, 0x90, 0x90, 0x90,
      0xE0, // D
      0xF0, 0x80, 0xF0, 0x80,
      0xF0, // E
      0xF0, 0x80, 0xF0, 0x80,
      0x80, // F

  };
  memcpy(&c->memory[chip8_font_addr], fontset, sizeof fontset);

  c->draw_flag = true; // draw once after init (all black_
}

bool chip8_load_rom(Chip8 *c, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;

  size_t dst = 0x0200; // program load address
  size_t cap = sizeof(c->memory) - dst;
  size_t n = fread(&c->memory[dst], 1, cap, f);
  fclose(f);

  return n > 0;
}

void chip8_cycle(
    Chip8 *c) { // CPU step of the emulator, from this void function, we can
                // read the next instruction, understand what it means, and
                // peform the correct action
  // this loop will run over and over, typically 60 times per second so the CPU
  // can keep updating registers, drawing sprites and responding to input

  // 1) Fetch opcode: read two bytes at c --> pc and advance pc by 2
  c->opcode = (c->memory[c->pc] << 8) | c->memory[c->pc + 1];
  c->pc += 2;

  // Think of this as grabbing two bytes from memory, sticking them together
  // into one 16-bit value, and storing it as an "opcode"
  // for example: memory[200] = 0x6A, memory[201] = 0x1E --> opcode = 0x6A1E
  // then move pc ahead by 2 so it points to the next instruction

  // 2) Decode opcode: pull out nnn, n, x, y, kk from the opcode
  uint16_t op = c->opcode;      // op = 0x6A1E
  uint16_t nnn = op & 0x0FFF;   // lowest 12 bits, 0xA1E
  uint8_t kk = op & 0x00FF;     // lowest 8 bits, 0x1E
  uint8_t n = op & 0x000F;      // lowest 4 bits, 0xE
  uint8_t x = (op >> 8) & 0x0F; // second nibble, 0xA
  uint8_t y = (op >> 4) & 0x0F; // third nibble, 0x1

  // Silence "unused variable" warnings for now:
  (void)n;
  (void)y;

  // Since now you have a 16 bit value, you need to pull it apart to see what
  // kind of instruction it is There are 4 groups of 4 bits, which are called
  // nibbles, each nibble means something different For the Chip-8, the first
  // nibble (6) tells what the instruction is the second nibble (A) is which
  // register it is, referring to (Vx) the last byte (1E) is the value to load

  // 3) Execute opcode: handle a few opcodes (CLS, LD, Vx, LD I, FX29, DXYN)
  // - 00E0: Clear gfx[], set draw_flag to true --> clears the screen, setting
  // drawflag to true so that the screen gets redrawn
  // - 6XKK: V[x] = kk --> set register Vx to the value kk
  // - ANNN: I = nnn --> set index register to nnn
  // - FX29: I = 0x050 + (V[x] & 0x0F) * 5 --> set I to the location of the
  // sprite for digit Vx
  // - DXYN: XOR - draw N rows from memory at location I to gfx at (Vx, Vy), set
  // draw_flag to true

  switch (op & 0xF000) {
  case 0x0000:
    switch (op) {
    case 0x00E0: // 00E0: Clear screen/display
      memset(c->gfx, 0, sizeof(c->gfx));
      c->draw_flag = true;
      break;

    case 0x00EE: // 00EE: Return from subroutine
      c->sp--;
      c->pc = c->stack[c->sp];
      break;

    default:
      // Some CHIP-8 variants use 0x00CN (scroll down N lines) and other opcodes
      // For now, just skip unknown 0x00** opcodes
      // printf("Unknown 0x00** opcode: 0x%04X at PC = 0x%3X\n", op, c->pc);
      break;
    }
    break;

  case 0x1000: // 1NNN: Jump to address NNN
    c->pc = nnn;
    break;

  case 0x2000: // 2NNN: Call subroutine at adresss NNN
    c->stack[c->sp] =
        c->pc; // store current pc on stack, after the assignment (=) is what we
               // want to store, and before it is where we want to store it
    c->sp++;   // increment the stack pointer so that we don't overwrite the
               // current stack
    c->pc = nnn; // jump to the subroutine address
    break;

  case 0x6000: // 6XKK: Set Vx = KK
    c->V[x] = kk;
    break;

  case 0x3000: // 3XKK: Skip next instruction if Vx == kk
    if (c->V[x] == kk)
      c->pc += 2;
    break;

  case 0x4000: // 4XKK: Skip next instruction if Vx != kk
    if (c->V[x] != kk)
      c->pc += 2;
    break;

  case 0x5000: // 5XY0: Skip next instruction if Vx == Vy
    if ((op & 0x000F) == 0) {
      if (c->V[x] == c->V[y])
        c->pc += 2;
    } else {
      printf("Unknown 5XY*: 0x%04X at PC = 0x%03X\n", op, c->pc);
    }
    break;

  case 0x7000: // 7XKK: Add KK to Vx
    c->V[x] += kk;
    break;

  case 0x8000: {
    uint8_t x = (op & 0x0F00) >> 8;
    uint8_t y = (op & 0x00F0) >> 4;

    switch (op & 0x000F) {
    case 0x0000: // 8XY0: Vx = Vy
      c->V[x] = c->V[y];
      break;

    case 0x0001: // 8XY1: Vx = Vx | Vy
      c->V[x] |= c->V[y];
      break;

    case 0x0002: // 8XY2: Vx = Vx & Vy
      c->V[x] &= c->V[y];
      break;

    case 0x0003: // 8XY3: Vx = Vx ^ Vy
      c->V[x] ^= c->V[y];
      break;

    case 0x0004: {
      uint16_t s = (uint16_t)c->V[x] + (uint16_t)c->V[y];
      c->V[0x000F] = (s > 0xFF);       // carry flag
      c->V[x] = (uint8_t)(s & 0x00FF); // 8-bit wrap
    } break;

    case 0x0005:                                   // 8XY5: Vx = Vx - Vy
      c->V[0x000F] = (c->V[x] >= c->V[y]) ? 1 : 0; // borrow flag
      c->V[x] -= c->V[y];
      break;

    case 0x0006:                     // 8XY6: Vx = Vx >> 1
      c->V[0x000F] = c->V[x] & 0x01; // least significant bit
      c->V[x] >>= 1;
      break;

    case 0x0007:                                   // 8XY7: Vx = Vy - Vx
      c->V[0x000F] = (c->V[y] >= c->V[x]) ? 1 : 0; // borrow flag
      c->V[x] = c->V[y] - c->V[x];
      break;

    case 0x000E:                            // 8XYE: Vx = Vx << 1
      c->V[0x000F] = (c->V[x] >> 7) & 0x01; // most significant bit
      c->V[x] <<= 1;
      break;

    default:
      printf("Unknown 8XY*: 0x%04X at PC = 0x%03X\n", op, c->pc);
      break;
    }
  } break;

  case 0x9000: // 9XY0: Skip next instruction if Vx != Vy
    if ((op & 0x000F) == 0) {
      if (c->V[x] != c->V[y])
        c->pc += 2;
    } else {
      printf("Unknown 9XY*: 0x%04X at PC = 0x%03X\n", op, c->pc);
    }
    break;

  case 0xB000: // BNNN: Jump to V0 + NNN
    c->pc = nnn + c->V[0];
    break;

  case 0xC000: // CXKK: Vx = random & KK
    c->V[x] = (rand() % 256) & kk;
    break;

  case 0xA000: // ANNN: Set I = NNN
    c->I = nnn;
    break;

  case 0xD000: { // DXYN: draw sprite at (Vx, Vy) that is N rows tall, and reads
                 // bytes from memory starting at I
    uint8_t x =
        (op & 0x0F00) >> 8; // masks op with 0x0F00 to get the second hex digit
                            // and then shifts down to get the register index x
    uint8_t y =
        (op & 0x00F0) >> 4; // masks op with 0x00F0 to get the third hex digit
                            // and then shifts down to get the register index y
    uint8_t n = (op & 0x000F); // masks op with 0x000F to get the last hex digit
                               // which is the height (number of rows)

    uint8_t vx =
        c->V[x]; // gives you the coordinates to draw at V[x], reads the value
                 // stored in the lines of code above, and gets the values
                 // stoerd in those registers of where to draw, for example if x
                 // in the top was equal to 3 you would get V[3]
    uint8_t vy = c->V[y]; // gives you the coordinates to draw at V[y]

    c->V[0x000F] = 0; // resets collision flag/register VF before drawing

    for (uint8_t row = 0; row < n;
         row++) { // a chip8 sprite is n rows tall, and each row is stored as
                  // one byte in memory starting at address I, this loop
                  // basiclaly says to go row by row through those n bytes
      uint8_t sprite_byte =
          c->memory[c->I + row]; // this reads one byte from memory and
                                 // continues to go until row is greater than I

      for (uint8_t bit = 0; bit < 8;
           bit++) { // inner loop walks through one sprite's rows of 8 bits from
                    // left to right and updates the pixels on screen
        if (sprite_byte &
            (0x0080 >>
             bit)) { // this is like a moving flashlight that checks each bit,
          uint8_t px =
              (uint8_t)((vx + bit) % 64); // this line of code moves to the
                                          // right as the bit increases,
          uint8_t py =
              (uint8_t)((vy + row) % 32); // this stays fixed for each row,
                                          // moves down as row increases
          uint16_t idx =
              (uint16_t)(py * 64 +
                         px); // this just converts the 2D screen coordinates
                              // into a 1D array index so that we can touch the
                              // right pixel in c -> gfx[]

          if (c->gfx[idx] == 1) // check if pixel is already set
            c->V[0x000F] = 1;   // set collision flag if so, basically says that
                              // the program is about to draw on top of a pixel
                              // that's already lit, set the flag to 1
          // if its already drawn and we draw again, the collision flag gets set
          // to 1 and basically cancels out the 2nd drawning and leaves the 1st
          // pixel on by itself

          c->gfx[idx] ^= 1; // XOR drawing
        }
      }

      c->draw_flag = true;
    }
  } break;

  case 0xE000: { // this opcode handles the skip if pressed and skip if not
                 // pressed logic
    uint8_t x = (op & 0x0F00) >> 8;
    uint8_t key =
        c->V[x] & 0x000F; // which chip8 key we are interested in (0-F)

    switch (op & 0x00FF) {
    case 0x009E:
      if (c->key[key])
        c->pc += 2;
      break; // pressed -> skip
    case 0x00A1:
      if (!c->key[key])
        c->pc += 2;
      break; // not pressed -> skip
    default:
      printf("Unknown EX**: 0x%04X at PC = 0x%03X\n", op, c->pc);
      break;
    }
  } break;

  case 0xF000: {
    uint8_t x = (op & 0x0F00) >> 8;

    switch (op & 0x00FF) {

    case 0x000A: { // this opcode waits for a key press and then stores the key
                   // in Vx
      int pressed = -1;
      for (int k = 0; k < 16; k++)
        if (c->key[k]) {
          pressed = k;
          break;
        }

      if (pressed < 0) {
        return;
      }

      c->V[x] = (uint8_t)pressed;
    } break;

    case 0x0029: {
      c->I =
          chip8_font_addr +
          (c->V[x] & 0x0F) * 5; // each sprite is 5 bytes long, so multiply by 5
    } break;

    case 0x0007: // FX07: Vx = delay_timer
      c->V[x] = c->delay_timer;
      break;

    case 0x0015: // FX15: delay_timer = Vx
      c->delay_timer = c->V[x];
      break;

    case 0x0018: // FX18: sound_timer = Vx
      c->sound_timer = c->V[x];
      break;

    case 0x001E: // FX1E: I = I + Vx
      c->I += c->V[x];
      break;

    case 0x0033: {
      uint8_t v = c->V[x];
      c->memory[c->I] = v / 100;           // hundreds place
      c->memory[c->I + 1] = (v / 10) % 10; // tens place
      c->memory[c->I + 2] = v % 10;        // ones place
      c->pc += 2;
    } break;

    case 0x0055: // FX55: Store V0-Vx to memory starting at I
      for (uint8_t i = 0; i <= x; i++) {
        c->memory[c->I + i] = c->V[i];
      }
      c->I += x + 1; // Some CHIP-8 implementations do this
      break;

    case 0x0065: // FX65: Load V0-Vx from memory starting at I
      for (uint8_t i = 0; i <= x; i++) {
        c->V[i] = c->memory[c->I + i];
      }
      c->I += x + 1; // Some CHIP-8 implementations do this
      break;

    default:
      printf("Unknown FX**: 0x%04X at PC = 0x%03X\n", op, c->pc);
      break;
    }
  } break;

  default:
    printf("Unknown opcode: 0x%04X at PC = 0x%03X\n", op, c->pc);
    break;
  }
}
