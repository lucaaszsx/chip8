#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "emulator.h"
#include "chip8.h"

int main(int argc, char **argv) {
    char *rom_path = NULL;
    struct Emulator emu;
    emu_init_config(&emu);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ips") == 0 && i + 1 < argc)
            emu.config.ips = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--rom-start") == 0 && i + 1 < argc)
            emu.config.rom_addr = strtoul(argv[++i], NULL, 0);
        else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc)
            emu.config.window.width = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc)
            emu.config.window.height = atoi(argv[++i]);
        else if (strcmp(argv[i], "--window") == 0 && i + 1 < argc) {
            sscanf(argv[++i], "%dx%d", &emu.config.window.width, &emu.config.window.height);
        } else if (strcmp(argv[i], "--freq") == 0 && i + 1 < argc)
            emu.config.audio.sample_rate = atoi(argv[++i]);
        else if (strcmp(argv[i], "--volume") == 0 && i + 1 < argc)
            emu.config.audio.volume = atof(argv[++i]);
        else if (strcmp(argv[i], "--hz") == 0 && i + 1 < argc)
            emu.config.audio.tone_hz = atof(argv[++i]);
        else if (strcmp(argv[i], "--amplitude") == 0 && i + 1 < argc)
            emu.config.audio.amplitude = atof(argv[++i]);
        else if (strcmp(argv[i], "--mute") == 0)
            emu.config.audio.mute = true;
        else if (argv[i][0] != '-')
            rom_path = argv[i];
        else {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    if (!rom_path) {
        fprintf(stderr, "You need to specify the ROM file to open. Usage: %s [options] PATH_TO_ROM\n", argv[0]);
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
