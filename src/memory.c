#include <stdio.h>
#include "memory.h"

void chip8_mem_reset(struct Chip8 *chip) {
    for (uint16_t addr = 0x0000; addr < sizeof(chip->mem); addr++)
        chip8_mem_write(chip, addr, 0x00);
}

void chip8_mem_write(struct Chip8 *chip, uint16_t addr, uint8_t word) {
    if (addr > 0xfff) {
        printf("attempt to write an out-of-range memory address\n");
        exit(1);
    }

    chip->mem[addr] = word;
}

void chip8_mem_write_many(struct Chip8 *chip, uint16_t start_addr, const uint8_t *words, size_t length) {
    for (size_t i = 0; i < length; i++)
        chip8_mem_write(chip, start_addr + i, words[i]);
}

uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr) {
    if (addr > 0xfff) {
        printf("attempt to access an out-of-range memory address\n");
        exit(1);
    }

    return chip->mem[addr];
}
