#ifndef EMULATOR_H
#define EMULATOR_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "chip8.h"

struct Emulator {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_AudioStream *audio_stream;
    double audio_g_phase;
    
    struct Chip8 *chip;
    bool beeping;
    bool running;
};

void emu_init(struct Emulator *emu, uint8_t *rom, size_t rom_size);
void emu_destroy(struct Emulator *emu);

#endif