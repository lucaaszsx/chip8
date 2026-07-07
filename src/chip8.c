#include "chip8.h"
#include "chip8_isa.h"
#include "timer.h"
#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define OPCODE_CLS 0x00e0
#define OPCODE_RTS 0x00ee

#define OPCODE_CAT_SYS    0x0
#define OPCODE_CAT_JMP    0x1
#define OPCODE_CAT_JSR    0x2
#define OPCODE_CAT_MOV    0x6
#define OPCODE_CAT_ADD    0x7
#define OPCODE_CAT_MVI    0xa
#define OPCODE_CAT_SPRITE 0xd

// -- addresses
const uint16_t PROG_ADDRESS = 0x0200;
const uint16_t FONT_ADDRESS = 0x50;

// -- constants
const uint8_t CHIP8_STD_FONT[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
    0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
    0x90, 0x90, 0xf0, 0x10, 0x10, // 4
    0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
    0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
    0xf0, 0x10, 0x20, 0x40, 0x40, // 7
    0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
    0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
    0xf0, 0x90, 0xf0, 0x90, 0x90, // A
    0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
    0xf0, 0x80, 0x80, 0x80, 0xf0, // C
    0xe0, 0x90, 0x90, 0x90, 0xe0, // D
    0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
    0xf0, 0x80, 0xf0, 0x80, 0x80  // F
};

// -- internals
static void chip8_cleanup(struct Chip8 *chip);
static void chip8_cycle(struct Chip8 *chip);
static uint8_t chip8_fetch_byte(struct Chip8 *chip);
static uint16_t chip8_fetch_word(struct Chip8 *chip);

static void chip8_mem_init(struct Chip8 *chip);
static void chip8_mem_write(struct Chip8 *chip, uint16_t addr, uint8_t word);
static void chip8_mem_write_many(struct Chip8 *chip, uint16_t start_addr, const uint8_t *words, size_t length);
static uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr);

static void print_state(struct Chip8 *chip) {
    printf("\033[2J\033[H");
    printf("---------- Chip-8 ----------\n");
    printf("REGISTERS:\n");
    printf("  PC (program counter): 0x%x\n", chip->pc);
    printf("  I (index): 0x%x\n", chip->i);
    printf("  General Purpose Registers (GPR):\n");

    for (size_t i = 0; i < sizeof(chip->v); i++) {
        if (i % 8 == 0) {
            if (i > 0) printf("\n");
            printf("    ");
        }
        
        printf("V%zu=0x%x    ", i, chip->v[i]);
    }
    printf("\n");

    printf("MEMORY (512..1024):\n");
    for (size_t i = 512; i < 1024; i++) {
        if (i % 64 == 0) {
            if (i > 0) printf("\n");
            printf("  ");
        }
        printf("%x ", chip->mem[i]);
    }
    printf("\n");
}

// -- public chip8 interface
void chip8_init(struct Chip8 *chip, size_t ips) {
    chip->ips = ips;
    chip->pc = PROG_ADDRESS;
    chip->i = 0x0000;
    chip->stack_index = 0;

    // cleanup and then writes the standard font in memory
    chip8_cleanup(chip);
    chip8_mem_write_many(chip, FONT_ADDRESS, CHIP8_STD_FONT, sizeof(CHIP8_STD_FONT) / sizeof(uint8_t));
}

void chip8_load_prog(struct Chip8 *chip, const uint8_t *prog, size_t prog_length) {
    chip8_mem_write_many(chip, PROG_ADDRESS, prog, prog_length);
}

void chip8_loop(struct Chip8 *chip) {
    const uint64_t interval = 1000000000 / chip->ips; // instructions per ns
    uint64_t next_tick = get_ticks();
    bool running = true;

    while (chip->pc != 0) {
        if (get_ticks() >= next_tick) {
            chip8_cycle(chip);
            next_tick += interval;
            print_state(chip);
        }
    }
}

// -- chip8 internals
void chip8_cleanup(struct Chip8 *chip) {
    for (size_t reg = 0; reg < sizeof(chip->v); reg++) chip->v[reg] = 0x00;
    for (size_t i = 0; i < sizeof(chip->stack) / sizeof(uint16_t); i++) chip->stack[i] = 0;

    chip8_mem_init(chip);
}

void chip8_cycle(struct Chip8 *chip) {
    uint16_t opcode = chip8_fetch_word(chip);

    if (opcode == OPCODE_CLS) {
        chip8_isa_cls(chip);
        return;
    }

    // gets the opcode category with the mask
    switch ((opcode & 0xf000) >> 12) {
        case OPCODE_CAT_SYS: {
            switch (opcode) {
                case OPCODE_CLS:
                    chip8_isa_cls(chip);
                    break;
                case OPCODE_RTS:
                    chip8_isa_rts(chip);
                    break;
            }
            break;
        }

        case OPCODE_CAT_JMP:
            chip8_isa_jmp(chip, opcode);
            break;

        case OPCODE_CAT_JSR:
            chip8_isa_jsr(chip, opcode);
            break;

        case OPCODE_CAT_MOV:
            chip8_isa_mov(chip, opcode);
            break;

        case OPCODE_CAT_ADD:
            chip8_isa_add(chip, opcode);
            break;

        case OPCODE_CAT_MVI:
            chip8_isa_mvi(chip, opcode);
            break;

        case OPCODE_CAT_SPRITE:
            chip8_isa_sprite(chip, opcode);
            break;

        default:
            // printf("invalid opcode %x", opcode);
            break;
    }
}

uint8_t chip8_fetch_byte(struct Chip8 *chip) {
    uint8_t byte = chip8_mem_read(chip, chip->pc);
    chip->pc = (chip->pc + 1) & 0xfff;
    return byte;
}

uint16_t chip8_fetch_word(struct Chip8 *chip) {
    uint16_t word = chip8_fetch_byte(chip) << 8;
    word |= chip8_fetch_byte(chip);
    return word;
}

// -- memory internals
void chip8_mem_init(struct Chip8 *chip) {
    for (uint16_t addr = 0x0000; addr < sizeof(chip->mem); addr++)
        chip8_mem_write(chip, addr, 0x00);
}

void chip8_mem_write(struct Chip8 *chip, uint16_t addr, uint8_t word) {
    if (addr > 0xffff) {
        printf("attempt to access an out-of-range memory address\n");
        exit(1);
    }

    chip->mem[addr] = word;
}

void chip8_mem_write_many(struct Chip8 *chip, uint16_t start_addr, const uint8_t *words, size_t length) {
    for (size_t i = 0; i < length; i++)
        chip8_mem_write(chip, start_addr + i, words[i]);
}

uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr) {
    return chip->mem[addr];
}
