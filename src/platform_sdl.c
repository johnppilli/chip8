#include "platform.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static SDL_AudioDeviceID audio_dev = 0;
static bool beeping = false;
static bool should_quit_flag = false;

static const int W = 64;
static const int H = 32;
static const int SCALE = 12;

static int16_t beep_buf[44100 / 60];


void platform_init(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);

    win = SDL_CreateWindow("CHIP-8",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            W * SCALE, H * SCALE, 0);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    SDL_AudioSpec want = {0}; 
    want.freq = 44100;
    want.format = AUDIO_S16SYS; 
    want.channels = 1;
    want.samples = 512;
    want.callback = NULL;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (audio_dev) {
        SDL_PauseAudioDevice(audio_dev, 1);
    }
    
    const int BEEP_SAMPLES = 44100 / 60;
    const int HALF_PERIOD = 44100 / 440 / 2;
    for (int i = 0; i < BEEP_SAMPLES; i++) {
        beep_buf[i] = ((i / HALF_PERIOD) % 2) ? 3000 : -3000; 
    }

}

void platform_shutdown(void) {
    if (audio_dev)
        SDL_CloseAudioDevice(audio_dev);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

bool platform_should_quit(void) {
    return should_quit_flag;
}

static int host_to_chip8(SDL_Keycode k) {
    switch (k) {
    case SDLK_1: return 0x1;
    case SDLK_2: return 0x2;
    case SDLK_3: return 0x3;
    case SDLK_4: return 0xC;
    case SDLK_q: return 0x4;
    case SDLK_w: return 0x5;
    case SDLK_e: return 0x6;
    case SDLK_r: return 0xD;
    case SDLK_a: return 0x7;
    case SDLK_s: return 0x8;
    case SDLK_d: return 0x9;
    case SDLK_f: return 0xE;
    case SDLK_z: return 0xA;
    case SDLK_x: return 0x0;
    case SDLK_c: return 0xB;
    case SDLK_v: return 0xF;
    default: return -1;
    }
}


void platform_poll_input(uint8_t *keys) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            should_quit_flag = true;
        } else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
            int k = host_to_chip8(e.key.keysym.sym);
            if (k >= 0)
                keys[k] = 1;
        } else if (e.type == SDL_KEYUP) {
            int k = host_to_chip8(e.key.keysym.sym);
            if (k >= 0)
                keys[k] = 0;
        }
    }
}




void platform_draw(const uint8_t *gfx) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (gfx[y * W + x]) {
                SDL_Rect px = {x * SCALE, y * SCALE, SCALE, SCALE};
                SDL_RenderFillRect(ren, &px); 
           }
        }
    }

    SDL_RenderPresent(ren);
}

void platform_beep(bool on) {
    if(!audio_dev) return; 

    if (on && !beeping) {
        beeping = true; 
        SDL_ClearQueuedAudio(audio_dev);
        SDL_PauseAudioDevice(audio_dev, 0);
    }
    if (on) {
        SDL_QueueAudio(audio_dev, beep_buf, sizeof(beep_buf));
    }
    if (!on && beeping) {
        beeping = false;
        SDL_PauseAudioDevice(audio_dev, 1);
        SDL_ClearQueuedAudio(audio_dev);
    }
}





