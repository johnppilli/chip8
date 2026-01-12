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

int main() {

  printf("Hello\n");

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
  const int W = 64, H = 32, SCALE = 12;

  SDL_Window *win =
      SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       W * SCALE, H * SCALE, 0);
  SDL_Renderer *r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  Chip8 chip;        // make the emulator state
  chip8_init(&chip); // initialize (pc = 0x200, font --> RAM)
  // load a rom here once, e.g., load a .ch8 into memory at 0x200
  if (!chip8_load_rom(&chip, "roms/PONG.ch8")) {
    SDL_Log("Failed to load ROM: roms/PONG.ch8");
    printf("Trying to load a different ROM...\n");
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    SDL_Quit();
  }
  printf("ROM loaded: PONG.ch8\n");

  Uint32 last_timer_tick = SDL_GetTicks();
  const Uint32 TIMER_MS = 1000 / 60; // 60 Hz

  int running = 1;
  uint16_t last_pc = 0;
  int same_pc_count = 0;
  int cycle_count = 0;
  printf("Starting emulator loop...\n");
  while (running) {
    handle_input(&chip, &running);
    cycle_count++;
    // chip8_cycle(&chip); //emulate one cycle
    // chip8_cycle(&chip); //emulate one cycle
    // chip8_cycle(&chip); //emulate one cycle
    // chip8_cycle(&chip); //emulate one cycle
    // chip8_cycle(&chip); //emulate one cycle
    // chip8_cycle(&chip); //emulate one cycle
    // chip8_cycle(&chip); //emulate one cycle

    for (int i = 0; i < 8; i++) {
      chip8_cycle(&chip);
    }

    // Tick Chip-8 timers at 60 Hz
    // Uint64 now = SDL_GetTicks64(); //asks SDL for the number of milliseconds
    // since SDL was initialized while (now - last_timer_tick >= TIMER_MS) {
    // //enters a loop if at least 60 Hz tick has elapsed since we last updated
    // timers if (chip.delay_timer > 0) chip.delay_timer--; //Decrements the
    // delay timer, but never below 0 if (chip.sound_timer > 0) { // same thing
    // as delay timer, but if you hook up audio, you would keep a tone playing
    // while sound_timer > 0 and stop when it hits 0
    //   chip.sound_timer--;
    //  (optional to do) play a beep sound when sound_timer > 0

    // last_timer_tick += TIMER_MS; // advances the "we handled a tick" time
    // forward by exactly one tick
    //  }
    // }
    Uint32 now = SDL_GetTicks();
    if (now - last_timer_tick >= TIMER_MS) {
      if (chip.delay_timer > 0)
        --chip.delay_timer;
      if (chip.sound_timer > 0)
        --chip.sound_timer;
      last_timer_tick = now;
    }

    // --- RENDER: draw the 64x32 CHIP-8 framebuffer ---
    if (chip.draw_flag) { // optional: only redraw when gfx changed
      printf("Drawing! PC: 0x%03X, Cycles: %d\n", chip.pc, cycle_count);

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

    // Debug: print every 1000 cycles to show we're running
    if (cycle_count % 1000 == 0) {
      printf("Running... Cycles: %d, PC: 0x%03X, draw_flag: %d\n", cycle_count,
             chip.pc, chip.draw_flag);
    }

    SDL_Delay(16);
  }
  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
