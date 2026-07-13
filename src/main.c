#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "renderer.h"
#include "keypad.h"
#include "chip8.h"

#define NS_PER_SEC      1000000000ULL
#define IPS             700                // instructions p/s

#define CYCLE_INTERVAL  (NS_PER_SEC / IPS) // IPS Hz
#define TIMERS_INTERVAL (NS_PER_SEC / 60)  // 60Hz
#define RENDER_INTERVAL (NS_PER_SEC / 60)  // 60Hz

#define AUDIO_FREQ      44100
#define TONE_HZ         440.0
#define TONE_AMPLITUDE  6000

static int8_t map_scancode(SDL_Scancode code);
static void on_cycle(struct Chip8 *chip);
static void SDLCALL audio_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

static double g_phase = 0.0;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "You need to specify the ROM file to open. Usage: %s PATH_TO_ROM", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *rom_file = fopen(argv[1], "rb");
    if (rom_file == NULL) {
        perror("Error while opening the ROM file");
        return EXIT_FAILURE;
    }

    fseek(rom_file, 0, SEEK_END);
    long rom_size = ftell(rom_file);
    rewind(rom_file);

    uint8_t *rom = (uint8_t *)(malloc(rom_size + 1));
    if (rom == NULL) {
        perror("Memory allocation for ROM failed");
        fclose(rom_file);
        return EXIT_FAILURE;
    }

    size_t bytes_read = fread(rom, 1, rom_size, rom_file);
    rom[bytes_read] = '\0';
    
    struct Chip8 chip;
    chip8_init(&chip);
    chip8_load_rom(&chip, rom, rom_size);
    chip.on_cycle = on_cycle;
    
    struct Chip8Renderer rd;
    chip8_renderer_init(&rd, &chip);

    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL_Init(AUDIO) failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_AudioSpec audio_spec = {
        .format = SDL_AUDIO_S16,
        .channels = 1,
        .freq = AUDIO_FREQ,
    };

    SDL_AudioStream *audio_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, audio_callback, NULL);
    if (audio_stream == NULL) {
        fprintf(stderr, "SDL_OpenAudioDeviceStream failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    bool beeping = false;

    uint64_t next_cycle_tick = SDL_GetTicksNS();
    uint64_t next_timers_tick = SDL_GetTicksNS();
    uint64_t next_render_tick = SDL_GetTicksNS();

    SDL_Event event;
    bool running = true;
    
    while (true) {
        exit:
            if (!running) break;
        
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_KEY_DOWN: {
                    int8_t sc = map_scancode(event.key.scancode);
                    if (sc > -1) chip8_keypad_press(chip.keypad, sc);
                    break;
                }

                case SDL_EVENT_KEY_UP: {
                    int8_t sc = map_scancode(event.key.scancode);
                    if (sc > -1) chip8_keypad_release(chip.keypad, sc);
                    break;
                }

                case SDL_EVENT_QUIT:
                    running = false;
                    goto exit;
                    break;
            }
        }

        uint64_t now = SDL_GetTicksNS();
        
        if (now >= next_cycle_tick) {
            chip8_cycle(&chip);
            next_cycle_tick += CYCLE_INTERVAL;
        }
        if (now >= next_timers_tick) {
            chip8_update_timers(&chip);
            next_timers_tick += TIMERS_INTERVAL;

            if (chip.st > 0 && !beeping) {
                SDL_ResumeAudioStreamDevice(audio_stream);
                beeping = true;
            } else if (chip.st == 0 && beeping) {
                SDL_PauseAudioStreamDevice(audio_stream);
                SDL_ClearAudioStream(audio_stream);
                beeping = false;
            }
        }
        if (now >= next_render_tick) {
            chip8_renderer_render(&rd);
            next_render_tick += RENDER_INTERVAL;
        }

        uint64_t next = next_cycle_tick;
        if (next_timers_tick < next) next = next_timers_tick;
        if (next_render_tick < next) next = next_render_tick;

        now = SDL_GetTicksNS();
        if (next > now) SDL_DelayNS(next - now); 
    }

    SDL_DestroyAudioStream(audio_stream);
    chip8_renderer_destroy(&rd);
    chip8_destroy(&chip);

    return EXIT_SUCCESS;
}

static void SDLCALL audio_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    (void)userdata;
    (void)total_amount;
    if (additional_amount <= 0) return;

    int samples_needed = additional_amount / sizeof(int16_t);
    int16_t *samples = (int16_t *)malloc(samples_needed * sizeof(int16_t));
    if (samples == NULL) return;

    double phase_inc = (4.0 * M_PI * TONE_HZ) / AUDIO_FREQ;
    for (int i = 0; i < samples_needed; i++) {
        samples[i] = (int16_t)(SDL_sin(g_phase) * TONE_AMPLITUDE);
        g_phase += phase_inc;
        if (g_phase >= 4.0 * M_PI) g_phase -= 4.0 * M_PI;
    }

    SDL_PutAudioStreamData(stream, samples, samples_needed * sizeof(int16_t));
    free(samples);
}

int8_t map_scancode(SDL_Scancode code) {
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

static void on_cycle(struct Chip8 *chip) {
    static int first_call = 1;
    static char buf[8192];
    size_t len = 0;

    if (first_call) {
        len += snprintf(buf + len, sizeof(buf) - len, "\033[2J");
        first_call = 0;
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\033[H");
    len += snprintf(buf + len, sizeof(buf) - len, "---------- Chip-8 ----------\n");
    len += snprintf(buf + len, sizeof(buf) - len, "REGISTERS:\n");
    len += snprintf(buf + len, sizeof(buf) - len, "  PC (program counter): 0x%x\n", chip->pc);
    len += snprintf(buf + len, sizeof(buf) - len, "  Byte at 0x%x: 0x%x\n", chip->pc, chip->mem[chip->pc]);
    len += snprintf(buf + len, sizeof(buf) - len, "  Byte at 0x%x+1: 0x%x\n", chip->pc, chip->mem[chip->pc + 1]);
    len += snprintf(buf + len, sizeof(buf) - len, "  I (index): 0x%x\n", chip->i);
    len += snprintf(buf + len, sizeof(buf) - len, "  DT: 0x%x\n", chip->dt);
    len += snprintf(buf + len, sizeof(buf) - len, "  ST: 0x%x\n", chip->st);
    len += snprintf(buf + len, sizeof(buf) - len, "  Keypad:\n");

    for (size_t i = 0; i < sizeof(chip->keypad->keys); i++) {
        if (i % 3 == 0) {
            if (i > 0) len += snprintf(buf + len, sizeof(buf) - len, "\n");
            len += snprintf(buf + len, sizeof(buf) - len, "    ");
        }
        len += snprintf(buf + len, sizeof(buf) - len, "%x=%x    ", i, chip8_keypad_pressed(chip->keypad, i));
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\n");

    len += snprintf(buf + len, sizeof(buf) - len, "  General Purpose Registers (GPR):\n");
    for (size_t i = 0; i < sizeof(chip->v); i++) {
        if (i % 8 == 0) {
            if (i > 0) len += snprintf(buf + len, sizeof(buf) - len, "\n");
            len += snprintf(buf + len, sizeof(buf) - len, "    ");
        }
        len += snprintf(buf + len, sizeof(buf) - len, "V%02x=0x%02x    ", (uint8_t)i, chip->v[i]);
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\n");

    len += snprintf(buf + len, sizeof(buf) - len, "MEMORY (512..768):\n");
    for (size_t i = 512; i < 768; i++) {
        if (i % 32 == 0) {
            if (i > 512) len += snprintf(buf + len, sizeof(buf) - len, "\n");
            len += snprintf(buf + len, sizeof(buf) - len, "  ");
        }
        len += snprintf(buf + len, sizeof(buf) - len, i == chip->pc ? "\033[31;1;4m%02x\033[0m " : "%02x ", chip->mem[i]);
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\n\033[J");

    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}