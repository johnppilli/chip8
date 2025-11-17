// src/core.h

#pragma once //pragma once makes sure to include this file once per build
#include <stdint.h> // this includes the standard integer type definitions from the C standard library
#include <stdbool.h> // adding this so that 'bool' is recognized as true or false in C



// stdint.h gives precise integer types like 
// uint8_t --> unsigned 8-bit integer (0-255)
// uint16_t --> unsigned 16-bit integer (0-65535)



//Use struct when you need to model an object with multiple fields and create variables of it
// This defines the emulator's main data structure


typedef struct{ // Struct = custom data type that bundles together all the parts of the Chip 8's CPU into one neate object
    uint16_t opcode; //current instruction
    uint8_t memory [4096]; // 4 KB RAM
    uint8_t V[16]; // 16 Reg (V0 to VF) 16 littles storage boxes for 8 bit values
    uint16_t I; // Index Reg, pointer into memory 
    uint16_t pc; // Program Counter, addresses of the next instruction, keeps track of where we are in the program  
   
    
    uint8_t gfx[64 * 32]; // framebuffer (0/1 per pixel)
    uint8_t delay_timer; // Timers  
    uint8_t sound_timer; // Timers

    bool draw_flag; // this becomes set to true whenever gfx changes 
    
    uint16_t stack [16]; // returns addresses for subroutines, addresses are 12 bits so you cant do 8 bits, have to make it 16 
    uint8_t sp;   // stack pointer is just an indez of 0-15, so 8 bits is enough

    uint8_t key[16]; // HEX based keypad (0x0 - 0xF), single bit info, only stores whether or not its pressed, 1 or 0 



} Chip8;


//Use an enum when you need named constants (sizes, addresses, IDs), that the compiler can treat as compile-time constants (handy for array sizes, switch cases, etc.)
enum {
    chip8_mem_size = 4096,

    //Conventional Addresses
    chip8_font_addr = 0x050, //where we copy the 4x5 font sprites
    chip8_rom_start = 0x200, // where program starts memory
    
    chip8_font_bytes = 16 * 5, //16 hex digits x 5 bytes per sprite = 80 bytes




};


void chip8_init(Chip8 *c);
bool chip8_load_rom(Chip8 *c, const char *path);
void chip8_cycle(Chip8 *c);


