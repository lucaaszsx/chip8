#ifndef EMULATOR_H
#define EMULATOR_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "chip8.h"

#define APP_TITLE     "Chip8 Emulator"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 320
#define WINDOW_BG_COLOR ((SDL_Color){139, 172, 15, 255})
#define WINDOW_FG_COLOR ((SDL_Color){48, 98, 48, 255})

#define AUDIO_SAMPLE_RATE    44100
#define AUDIO_TONE_HZ        440.0
#define AUDIO_TONE_AMPLITUDE 0.5f

#define NS_PER_SEC      1000000000ULL
#define TIMERS_INTERVAL (NS_PER_SEC / 60)  // 60Hz
#define RENDER_INTERVAL (NS_PER_SEC / 60)  // 60Hz

#define DEFAULT_IPS 700 // instructions p/s

struct EmulatorConfig {
    uint32_t ips;
    uint16_t rom_addr;

    struct {
        int width;
        int height;
        SDL_Color bg_color;
        SDL_Color fg_color;
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