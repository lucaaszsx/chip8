#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

void chip8_mem_write(struct Chip8 *chip, uint16_t addr, uint8_t word) {
    if (addr > 0xfff) {
        fprintf(stderr, "attempt to write an out-of-range memory address: 0x%04x\n", addr);
        exit(EXIT_FAILURE);
    }

    chip->mem[addr] = word;
}

void chip8_mem_write_many(struct Chip8 *chip, uint16_t start_addr, const uint8_t *words, size_t length) {
    if ((size_t)start_addr + length > 0x1000) {
        fprintf(stderr, "attempt to write beyond memory: start=0x%04x, length=%zu\n", start_addr, length);
        exit(EXIT_FAILURE);
    }

    memcpy(&chip->mem[start_addr], words, length);
}

uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr) {
    if (addr > 0xfff) {
        fprintf(stderr, "attempt to access an out-of-range memory address: 0x%04x\n", addr);
        exit(EXIT_FAILURE);
    }

    return chip->mem[addr];
}
