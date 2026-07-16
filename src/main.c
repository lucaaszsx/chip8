#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "emulator.h"
#include "chip8.h"

static void show_help_message(FILE *stream, char *executable);
static int get_raw_color(SDL_Color color);

int main(int argc, char **argv) {
    // setup seed for generating random numbers
    srand(time(NULL));

    char *rom_path = NULL;
    struct Emulator emu;
    emu_init_config(&emu);

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] != '-') {
            rom_path = arg;
            continue;
        }
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            show_help_message(stdout, argv[0]);
            return EXIT_SUCCESS;
        }

        if (strcmp(arg, "--ips") == 0 && i + 1 < argc) {
            emu.config.ips = strtoul(argv[++i], NULL, 10);
        }
        else if (strcmp(arg, "--rom-start") == 0 && i + 1 < argc) {
            emu.config.rom_addr = strtoul(argv[++i], NULL, 0);
        }
        else if (strcmp(arg, "--width") == 0 && i + 1 < argc) {
            emu.config.window.width = atoi(argv[++i]);
        }
        else if (strcmp(arg, "--height") == 0 && i + 1 < argc) {
            emu.config.window.height = atoi(argv[++i]);
        }
        else if (strcmp(arg, "--window") == 0 && i + 1 < argc) {
            sscanf(argv[++i], "%dx%d", &emu.config.window.width, &emu.config.window.height);
        }
        else if (
            (
                strcmp(arg, "--bg") == 0 ||
                strcmp(arg, "--fg") == 0
            ) &&
            i + 1 < argc
        ) {
            int raw_color = strtoul(argv[++i], NULL, 16);
            SDL_Color color = {
                (raw_color & 0xff0000) >> 16, // r (RRggbbaa)
                (raw_color & 0xff00) >> 8,    // g (rrGGbbaa)
                raw_color & 0xff,             // b (rrggBBaa)
                255                           // a (rrggbbAA)
            };

            if (strcmp(arg, "--bg") == 0) emu.config.window.bg_color = color;
            else emu.config.window.fg_color = color;
        }
        else if (strcmp(arg, "--freq") == 0 && i + 1 < argc) {
            emu.config.audio.sample_rate = atoi(argv[++i]);
        }
        else if (strcmp(arg, "--volume") == 0 && i + 1 < argc) {
            emu.config.audio.volume = atof(argv[++i]);
        }
        else if (strcmp(arg, "--hz") == 0 && i + 1 < argc) {
            emu.config.audio.tone_hz = atof(argv[++i]);
        }
        else if (strcmp(arg, "--amplitude") == 0 && i + 1 < argc) {
            emu.config.audio.amplitude = atof(argv[++i]);
        }
        else if (strcmp(arg, "--mute") == 0) {
            emu.config.audio.mute = true;
        }
        else {
            fprintf(stderr, "Unknown or incorrectly used option: %s\n", arg);
            show_help_message(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (!rom_path) {
        fprintf(stderr, "You need to specify the ROM file to open.\n");
        show_help_message(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    FILE *rom_file = fopen(rom_path, "rb");
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

    emu_run(&emu, rom, rom_size);
    emu_destroy(&emu);

    return EXIT_SUCCESS;
}

static void show_help_message(FILE *stream, char *executable) {
    fprintf(
        stream,
        "Usage: %s [options] [rom]\n"
        "       %s [-h | --help]\n"
        "Options:\n"
        "    CPU:\n"
        "        --ips       Define the amount of instructions per second that emulator should run (default=%d)\n"
        "        --rom-start Define the start address ROM (default=0x%04x)\n"
        "    Window:\n"
        "        --width  Window width (default=%d)\n"
        "        --height Window height (default=%d)\n"
        "        --window Window size, e.g.: 512x256 (widthxheight) \n"
        "        --bg     Background color (default=0x%06X)\n"
        "        --fg     Foreground color (default=0x%06X)\n"
        "    Audio:\n"
        "        --freq      Audio frequency (default=%d)\n"
        "        --volume    Audio volume (default=1.0)\n"
        "        --hz        Audio tone HZ (default=%.2f)\n"
        "        --amplitude Audio amplitude (default=%.2f)\n\n"
        "    Flags:\n"
        "        --mute      If the audio should be muted\n\n"
        "Controls:\n"
        "    Keypad:\n"
        "        1 2 3 4\n"
        "        Q W E R\n"
        "        A S D F\n"
        "        Z X C V\n"
        "    ESC - Reset the emulator (be careful, your progress will be lost)\n\n",
        executable,
        executable,

        // CPU
        DEFAULT_IPS,
        ROM_ADDRESS,

        // Window
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        get_raw_color(WINDOW_BG_COLOR),
        get_raw_color(WINDOW_FG_COLOR),

        // Audio
        AUDIO_SAMPLE_RATE,
        AUDIO_TONE_HZ,
        AUDIO_TONE_AMPLITUDE
    );
}

static int get_raw_color(SDL_Color color) {
    return (color.r << 16) | (color.g << 8) | (color.b << 4) | color.a;
}