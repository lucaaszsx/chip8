#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "renderer.h"
#include "keypad.h"
#include "chip8.h"

#define NS_PER_SEC      1000000000ULL
#define IPS             700                // instructions p/s

#define CYCLE_INTERVAL  (NS_PER_SEC / IPS) // IPS Hz
#define TIMERS_INTERVAL (NS_PER_SEC / 60)  // 60Hz
#define RENDER_INTERVAL (NS_PER_SEC / 60)  // 60Hz

static int8_t map_scancode(SDL_Scancode code);
static void on_cycle(struct Chip8 *chip);

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

    chip8_renderer_destroy(&rd);
    chip8_destroy(&chip);

    return EXIT_SUCCESS;
}

int8_t map_scancode(SDL_Scancode code) {
    switch (code) {
        // row 1
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xc;

        // row 2
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xd;

        // row 3
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xe;

        // row 4
        case SDL_SCANCODE_Z: return 0xa;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xb;
        case SDL_SCANCODE_V: return 0xf;

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
