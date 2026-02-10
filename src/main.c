#include "core.h"
#include <SDL2/SDL.h>
#include <stdio.h>

static int host_to_chip8(SDL_Keycode k) {
  switch (k) {
  case SDLK_1:
    return 0x1;
  case SDLK_2:
    return 0x2;
  case SDLK_3:
    return 0x3;
  case SDLK_4:
    return 0xC;
  case SDLK_q:
    return 0x4;
  case SDLK_w:
    return 0x5;
  case SDLK_e:
    return 0x6;
  case SDLK_r:
    return 0xD;
  case SDLK_a:
    return 0x7;
  case SDLK_s:
    return 0x8;
  case SDLK_d:
    return 0x9;
  case SDLK_f:
    return 0xE;
  case SDLK_z:
    return 0xA;
  case SDLK_x:
    return 0x0;
  case SDLK_c:
    return 0xB;
  case SDLK_v:
    return 0xF;
  default:
    return -1;
  }
}

static void handle_input(Chip8 *c, int *running) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      *running = 0;
    } else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
      int k = host_to_chip8(e.key.keysym.sym);
      if (k >= 0)
        c->key[k] = 1;
    } else if (e.type == SDL_KEYUP) {
      int k = host_to_chip8(e.key.keysym.sym);
      if (k >= 0)
        c->key[k] = 0;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: chip8 <rom>\nExample: chip8 roms/TETRIS.ch8\n");
    return 1;
  }
  const char *rom_path = argv[1];

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
  const int W = 64, H = 32, SCALE = 12;

  SDL_Window *win =
      SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       W * SCALE, H * SCALE, 0);
  SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  Chip8 chip;
  chip8_init(&chip);
  if (!chip8_load_rom(&chip, rom_path)) {
    SDL_Log("Failed to load ROM: %s", rom_path);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  Uint32 last_timer_tick = SDL_GetTicks();
  const Uint32 TIMER_MS = 1000 / 60;

  int running = 1;
  while (running) {
    handle_input(&chip, &running);

    for (int i = 0; i < 8; i++) {
      chip8_cycle(&chip);
    }

    // Decrement timers at 60 Hz
    Uint32 now = SDL_GetTicks();
    if (now - last_timer_tick >= TIMER_MS) {
      if (chip.delay_timer > 0)
        --chip.delay_timer;
      if (chip.sound_timer > 0)
        --chip.sound_timer;
      last_timer_tick = now;
    }

    // Render framebuffer
    if (chip.draw_flag) {
      SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
      SDL_RenderClear(r);

      SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
          if (chip.gfx[y * 64 + x]) {
            SDL_Rect px = {x * SCALE, y * SCALE, SCALE, SCALE};
            SDL_RenderFillRect(r, &px);
          }
        }
      }

      SDL_RenderPresent(r);
      chip.draw_flag = false;
    }

    SDL_Delay(16);
  }
  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
