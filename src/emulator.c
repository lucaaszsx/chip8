#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "emulator.h"
#include "display.h"
#include "chip8.h"

#define APP_TITLE     "Chip8 Emulator"

#define NS_PER_SEC      1000000000ULL
#define TIMERS_INTERVAL (NS_PER_SEC / 60)  // 60Hz
#define RENDER_INTERVAL (NS_PER_SEC / 60)  // 60Hz

static void emu_reset(struct Emulator *emu);
static void emu_init_sdl(struct Emulator *emu);
static void emu_init_chip(struct Emulator *emu);
static void emu_render(struct Emulator *emu);
static int8_t map_scancode(SDL_Scancode code);
static void SDLCALL sdl_audio_cb(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

void emu_init_config(struct Emulator *emu) {
    // Default configuration for emulator
    emu->config.ips = DEFAULT_IPS;
    emu->config.rom_addr = ROM_ADDRESS;

    emu->config.window.width = WINDOW_WIDTH;
    emu->config.window.height = WINDOW_HEIGHT;
    emu->config.window.bg_color = WINDOW_BG_COLOR;
    emu->config.window.fg_color = WINDOW_FG_COLOR;

    emu->config.audio.sample_rate = AUDIO_SAMPLE_RATE;
    emu->config.audio.tone_hz = AUDIO_TONE_HZ;
    emu->config.audio.amplitude = AUDIO_TONE_AMPLITUDE;
    emu->config.audio.volume = 1.0f;
    emu->config.audio.mute = false;
}

void emu_run(struct Emulator *emu, uint8_t *rom, size_t rom_size) {
    // initializes SDL and Chip8
    emu_init_sdl(emu);
    emu_init_chip(emu);

    run_init:
    // loads the rom
    chip8_load_rom(emu->chip, rom, rom_size);

    // main loop
    emu->running = true;

    const Uint64 cycle_interval = NS_PER_SEC / emu->config.ips;

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
                    SDL_Scancode sc = event.key.scancode;

                        // esc -> resets the emulator
                    if (sc == SDL_SCANCODE_ESCAPE && !event.key.repeat) {
                        emu_reset(emu);
                        goto run_init;
                    }

                    int8_t key = map_scancode(sc);
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
            }
        }

        Uint64 now = SDL_GetTicksNS();
        
        if (now >= next_cycle_tick) {
            chip8_cycle(emu->chip);
            next_cycle_tick += cycle_interval;
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

        Uint64 next = next_cycle_tick;
        if (next_timers_tick < next) next = next_timers_tick;
        if (next_render_tick < next) next = next_render_tick;

        now = SDL_GetTicksNS();
        if (next > now) SDL_DelayNS(next - now); 
    }
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

static void emu_reset(struct Emulator *emu) {
    chip8_reset(emu->chip);
    emu->running = false;

    if (emu->beeping) {
        SDL_PauseAudioStreamDevice(emu->audio_stream);
        SDL_ClearAudioStream(emu->audio_stream);
        emu->beeping = false;
    }
    emu->audio_g_phase = 0;
}

static void emu_init_sdl(struct Emulator *emu) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init(AUDIO) failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window;
    if ((window = SDL_CreateWindow(APP_TITLE, emu->config.window.width, emu->config.window.height, 0)) == NULL) {
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
        .freq = emu->config.audio.sample_rate
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

static void emu_init_chip(struct Emulator *emu) {
    struct Chip8 *chip = malloc(sizeof(struct Chip8));
    if (chip == NULL) {
        fprintf(stderr, "Failed to allocate memory for the CHIP-8");
        exit(EXIT_FAILURE);
    }
    
    chip8_init(chip, emu->config.rom_addr);
    emu->chip = chip;
}

static void emu_render(struct Emulator *emu) {
    const int pixel_width = emu->config.window.width / DISPLAY_COLS;
    const int pixel_height = emu->config.window.height / DISPLAY_ROWS;
    const SDL_Color bg = emu->config.window.bg_color;
    const SDL_Color fg = emu->config.window.fg_color;

    SDL_SetRenderDrawColor(emu->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(emu->renderer);

    SDL_SetRenderDrawColor(emu->renderer, fg.r, fg.g, fg.b, fg.a);
    
    for (size_t i = 0; i < DISPLAY_SIZE; i++) {
        if (emu->chip->display->vram[i] != 1) continue;
        
        SDL_FRect rect = {
            .x=(i % DISPLAY_COLS) * pixel_width,
            .y=(int)(i / DISPLAY_COLS) * pixel_height,
            .w=pixel_width,
            .h=pixel_height
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

    if (emu->config.audio.mute) {
        SDL_memset(samples, 0, samples_needed * sizeof(int16_t));
        SDL_PutAudioStreamData(stream, samples, samples_needed * sizeof(int16_t));
        free(samples);
        return;
    }

    double phase_inc = (2.0 * M_PI * emu->config.audio.tone_hz) / emu->config.audio.sample_rate;
    float gain = emu->config.audio.amplitude * emu->config.audio.volume;
    
    for (int i = 0; i < samples_needed; i++) {
        samples[i] = (int16_t)(SDL_sin(emu->audio_g_phase) * gain * INT16_MAX);
        emu->audio_g_phase += phase_inc;
        if (emu->audio_g_phase >= 2.0 * M_PI) emu->audio_g_phase -= 2.0 * M_PI;
    }

    SDL_PutAudioStreamData(stream, samples, samples_needed * sizeof(int16_t));
    free(samples);
}
