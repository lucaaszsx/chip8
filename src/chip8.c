#include "chip8.h"
#include <stdio.h>

// addresses
const uint16_t PROG_ADDRESS = 0x0200;
const uint16_t FONT_ADDRESS = 0x50;

// constants
const uint8_t CHIP8_STD_FONT[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// internals
static uint8_t chip8_fetch_byte(struct Chip8 *chip);
static uint16_t chip8_fetch_word(struct Chip8 *chip);

static void chip8_mem_init(struct Chip8 *chip);
static void chip8_mem_write(struct Chip8 *chip, uint16_t addr, uint8_t word);
static uint16_t chip8_mem_write_many(struct Chip8 *chip, uint16_t start_addr, const uint8_t *words, size_t length);
static uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr);

// public chip8 interface
void chip8_init(struct Chip8 *chip) {
    chip8_mem_init(chip);

    // writes the standard font in memory
    uint16_t last_addr = chip8_mem_write_many(chip, FONT_ADDRESS, CHIP8_STD_FONT, sizeof(CHIP8_STD_FONT) / sizeof(uint8_t));

    chip->pc = last_addr;
    chip->i = 0x0000;

    for (size_t reg = 0; reg < 16; reg++) chip->v[reg] = 0x00;
}

void chip8_load_prog(struct Chip8 *chip, const uint8_t *prog, size_t prog_length) {
    chip8_mem_write_many(chip, PROG_ADDRESS, prog, prog_length);
}

void chip8_loop(struct Chip8 *chip) {
    uint16_t opcode;
    size_t i = 0;
    while (i < 4096) {
        opcode = chip8_fetch_word(chip);
        if (opcode != 0) printf("opcode fetched at %d: 0x%X\n", chip->pc - 2, opcode);
        i += 4;
    }
}

// chip8 internals
uint8_t chip8_fetch_byte(struct Chip8 *chip) {
    uint8_t byte = chip8_mem_read(chip, chip->pc);
    chip->pc = (chip->pc + 1) % 4095;
    return byte;
}

uint16_t chip8_fetch_word(struct Chip8 *chip) {
    uint16_t word = chip8_fetch_byte(chip) << 8;
    word |= chip8_fetch_byte(chip);
    return word;
}

// memory internals
void chip8_mem_init(struct Chip8 *chip) {
    for (uint16_t addr = 0x0000; addr < sizeof(chip->mem); addr++)
        chip8_mem_write(chip, addr, 0x00);
}

void chip8_mem_write(struct Chip8 *chip, const uint16_t addr, const uint8_t word) {
    chip->mem[addr] = word;
}

uint16_t chip8_mem_write_many(struct Chip8 *chip, const uint16_t start_addr, const uint8_t *words, const size_t length) {
    uint16_t curr_addr = start_addr;
    for (size_t i = 0; i < length; i++) {
        chip8_mem_write(chip, curr_addr, words[i]);
        curr_addr = curr_addr + 1;
    }

    return curr_addr;
}

uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr) {
    return chip->mem[addr];
}
