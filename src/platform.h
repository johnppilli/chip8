#pragma once
#include <stdbool.h>
#include <stdint.h>

void platform_init(void); //called once at program start, opens and sets up everything
void platform_shutdown(void); //called once at program end, closes everything

bool platform_should_quit(void); //returns true if user clicks x on teh window, false otherwise
void platform_poll_input(uint8_t *keys); //takes pointer to chip8's key[16] array and updates based on pressed 

void platform_draw(const uint8_t *gfx); //takes a pointer to teh 64x32 framebuffer and draws to screen 
void platform_beep(bool on); //starts/stops beep




