#include <stdlib.h>
#include <stdio.h>
#include "emulator.h"
#include "chip8.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "You need to specify the ROM file to open. Usage: %s PATH_TO_ROM\n", argv[0]);
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
    
    struct Emulator emu;
    emu_init(&emu, rom, rom_size);
    emu_destroy(&emu);
}
