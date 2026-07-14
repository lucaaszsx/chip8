#ifndef EMULATOR_H
#define EMULATOR_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "chip8.h"

struct EmulatorConfig {
    uint32_t ips;
    uint16_t rom_addr;

    struct {
        int width;
        int height;
    } window;

    struct {
        int sample_rate;
        double tone_hz;
        float volume;
        float amplitude;
        bool mute;
    } audio;
};

struct Emulator {
    struct EmulatorConfig config;

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_AudioStream *audio_stream;
    double audio_g_phase;
    
    struct Chip8 *chip;
    bool beeping;
    bool running;
};

void emu_init_config(struct Emulator *emu);
void emu_run(struct Emulator *emu, uint8_t *rom, size_t rom_size);
void emu_destroy(struct Emulator *emu);

#endif