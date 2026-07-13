#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "emulator.h"
#include "display.h"
#include "chip8.h"

#define APP_TITLE     "Chip8 Emulator"
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 320

#define PIXEL_WIDTH  ((int)(SCREEN_WIDTH / DISPLAY_COLS))
#define PIXEL_HEIGHT ((int)(SCREEN_HEIGHT / DISPLAY_ROWS))

#define AUDIO_FREQ     44100
#define TONE_HZ        440.0
#define TONE_AMPLITUDE 6000

#define NS_PER_SEC 1000000000ULL
#define IPS        700 // instructions p/s

#define CYCLE_INTERVAL  (NS_PER_SEC / IPS) // IPS Hz
#define TIMERS_INTERVAL (NS_PER_SEC / 60)  // 60Hz
#define RENDER_INTERVAL (NS_PER_SEC / 60)  // 60Hz

static void emu_init_sdl(struct Emulator *emu);
static void emu_init_chip(struct Emulator *emu, uint8_t *rom, size_t rom_size);
static void emu_start_loop(struct Emulator *emu);
static void emu_render(struct Emulator *emu);
static int8_t map_scancode(SDL_Scancode code);
static void SDLCALL sdl_audio_cb(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

void emu_init(struct Emulator *emu, uint8_t *rom, size_t rom_size) {
    emu_init_sdl(emu);
    emu_init_chip(emu, rom, rom_size);
    emu_start_loop(emu);
}

void emu_destroy(struct Emulator *emu) {
    chip8_destroy(emu->chip);
    
    if (emu->window) {
        SDL_DestroyWindow(emu->window);
        emu->window = NULL;
    }
    if (emu->renderer) {
        SDL_DestroyRenderer(emu->renderer);
        emu->renderer = NULL;
    }
    if (emu->audio_stream) {
        SDL_DestroyAudioStream(emu->audio_stream);
        emu->audio_stream = NULL;
    }
    
    SDL_Quit();
}

static void emu_init_sdl(struct Emulator *emu) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init(AUDIO) failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window;
    if ((window = SDL_CreateWindow(APP_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT, 0)) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL window initialization failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer *renderer;
    if ((renderer = SDL_CreateRenderer(window, NULL)) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL renderer initialization failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_AudioSpec audio_spec = {
        .format = SDL_AUDIO_S16,
        .channels = 1,
        .freq = AUDIO_FREQ
    };

    SDL_AudioStream *audio_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, sdl_audio_cb, emu);
    if (audio_stream == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_OpenAudioDeviceStream failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    emu->window = window;
    emu->renderer = renderer;
    emu->audio_stream = audio_stream;
    emu->audio_g_phase = 0;
}

static void emu_init_chip(struct Emulator *emu, uint8_t *rom, size_t rom_size) {
    struct Chip8 *chip = malloc(sizeof(struct Chip8));
    if (chip == NULL) {
        fprintf(stderr, "Failed to allocate memory for the CHIP-8");
        exit(EXIT_FAILURE);
    }
    
    chip8_init(chip);
    chip8_load_rom(chip, rom, rom_size);

    emu->chip = chip;
}

static void emu_start_loop(struct Emulator *emu) {
    emu->running = true;

    Uint64 next_cycle_tick = SDL_GetTicksNS();
    Uint64 next_timers_tick = SDL_GetTicksNS();
    Uint64 next_render_tick = SDL_GetTicksNS();

    SDL_Event event;
    while (true) {
        loop_start:
            if (!emu->running) break;

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_EVENT_KEY_DOWN: {
                        int8_t key = map_scancode(event.key.scancode);
                        if (key > -1) chip8_keypad_press(emu->chip->keypad, key);
                        break;
                    }
    
                    case SDL_EVENT_KEY_UP: {
                        int8_t key = map_scancode(event.key.scancode);
                        if (key > -1) chip8_keypad_release(emu->chip->keypad, key);
                        break;
                    }
    
                    case SDL_EVENT_QUIT:
                        emu->running = false;
                        goto loop_start;
                        break;
                }
            }
    
            Uint64 now = SDL_GetTicksNS();
            
            if (now >= next_cycle_tick) {
                chip8_cycle(emu->chip);
                next_cycle_tick += CYCLE_INTERVAL;
            }
            if (now >= next_timers_tick) {
                chip8_update_timers(emu->chip);
                next_timers_tick += TIMERS_INTERVAL;
    
                if (emu->chip->st > 0 && !emu->beeping) {
                    SDL_ResumeAudioStreamDevice(emu->audio_stream);
                    emu->beeping = true;
                } else if (emu->chip->st == 0 && emu->beeping) {
                    SDL_PauseAudioStreamDevice(emu->audio_stream);
                    SDL_ClearAudioStream(emu->audio_stream);
                    emu->beeping = false;
                }
            }
            if (now >= next_render_tick) {
                emu_render(emu);
                next_render_tick += RENDER_INTERVAL;
            }
    
            uint64_t next = next_cycle_tick;
            if (next_timers_tick < next) next = next_timers_tick;
            if (next_render_tick < next) next = next_render_tick;
    
            now = SDL_GetTicksNS();
            if (next > now) SDL_DelayNS(next - now); 
    }
}

static void emu_render(struct Emulator *emu) {
    SDL_SetRenderDrawColor(emu->renderer, 139, 172, 15, 255);
    SDL_RenderClear(emu->renderer);

    SDL_SetRenderDrawColor(emu->renderer, 48, 98, 48, 255);
    
    for (size_t i = 0; i < DISPLAY_SIZE; i++) {
        if (emu->chip->display->vram[i] != 1) continue;
        
        SDL_FRect rect = {
            .x=(i % DISPLAY_COLS) * PIXEL_WIDTH,
            .y=(int)(i / DISPLAY_COLS) * PIXEL_HEIGHT,
            .w=PIXEL_WIDTH,
            .h=PIXEL_HEIGHT
        };
        SDL_RenderFillRect(emu->renderer, &rect);
    }

    SDL_RenderPresent(emu->renderer);
}

static int8_t map_scancode(SDL_Scancode code) {
    switch (code) {
        // row 1
        case SDL_SCANCODE_1: return KEYPAD_KEY_1;
        case SDL_SCANCODE_2: return KEYPAD_KEY_2;
        case SDL_SCANCODE_3: return KEYPAD_KEY_3;
        case SDL_SCANCODE_4: return KEYPAD_KEY_C;

        // row 2
        case SDL_SCANCODE_Q: return KEYPAD_KEY_4;
        case SDL_SCANCODE_W: return KEYPAD_KEY_5;
        case SDL_SCANCODE_E: return KEYPAD_KEY_6;
        case SDL_SCANCODE_R: return KEYPAD_KEY_D;

        // row 3
        case SDL_SCANCODE_A: return KEYPAD_KEY_7;
        case SDL_SCANCODE_S: return KEYPAD_KEY_8;
        case SDL_SCANCODE_D: return KEYPAD_KEY_9;
        case SDL_SCANCODE_F: return KEYPAD_KEY_E;

        // row 4
        case SDL_SCANCODE_Z: return KEYPAD_KEY_A;
        case SDL_SCANCODE_X: return KEYPAD_KEY_0;
        case SDL_SCANCODE_C: return KEYPAD_KEY_B;
        case SDL_SCANCODE_V: return KEYPAD_KEY_F;

        default: return -1;
    }
}

static void SDLCALL sdl_audio_cb(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    (void)total_amount;
    if (additional_amount <= 0) return;

    int samples_needed = additional_amount / sizeof(int16_t);
    int16_t *samples = (int16_t *)malloc(samples_needed * sizeof(int16_t));
    if (samples == NULL) return;

    struct Emulator *emu = (struct Emulator*)userdata;

    double phase_inc = (4.0 * M_PI * TONE_HZ) / AUDIO_FREQ;
    for (int i = 0; i < samples_needed; i++) {
        samples[i] = (int16_t)(SDL_sin(emu->audio_g_phase) * TONE_AMPLITUDE);
        emu->audio_g_phase += phase_inc;
        if (emu->audio_g_phase >= 4.0 * M_PI) emu->audio_g_phase -= 4.0 * M_PI;
    }

    SDL_PutAudioStreamData(stream, samples, samples_needed * sizeof(int16_t));
    free(samples);
}
