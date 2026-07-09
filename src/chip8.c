#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "display.h"
#include "memory.h"
#include "chip8.h"
#include "isa.h"

#define OPCODE_CLS 0x00e0
#define OPCODE_RTS 0x00ee

// -- addresses
const uint16_t ROM_ADDRESS = 0x0200;
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
static uint8_t chip8_fetch_byte(struct Chip8 *chip);
static uint16_t chip8_fetch_word(struct Chip8 *chip);

// -- public chip8 interface
void chip8_init(struct Chip8 *chip) {
    chip->pc = ROM_ADDRESS;
    chip->i = 0x0000;
    chip->stack_index = 0;
    chip->display = malloc(sizeof(struct Chip8Display));
    chip->on_cycle = NULL;
    
    // cleanup and then writes the standard font in memory
    chip8_reset(chip);
    chip8_mem_write_many(chip, FONT_ADDRESS, CHIP8_STD_FONT, sizeof(CHIP8_STD_FONT));
}

void chip8_reset(struct Chip8 *chip) {
    memset(chip->v, 0, sizeof(chip->v));
    memset(chip->stack, 0, sizeof(chip->stack) / sizeof(uint16_t));
    memset(chip->mem, 0, sizeof(chip->mem));
    chip8_display_reset(chip->display);
}

void chip8_load_rom(struct Chip8 *chip, const uint8_t *rom, size_t rom_length) {
    chip8_mem_write_many(chip, ROM_ADDRESS, rom, rom_length);
}

void chip8_cycle(struct Chip8 *chip) {
    uint16_t opcode = chip8_fetch_word(chip);
    bool matches = true;
    
    // gets the opcode category with the mask
    switch ((opcode & 0xf000) >> 12) {
        case 0x0: {
            switch (opcode) {
                case OPCODE_CLS:
                    chip8_isa_cls(chip);
                    break;
                case OPCODE_RTS:
                    chip8_isa_rts(chip);
                    break;
                default:
                    matches = false;
            }
            break;
        }

        case 0x1:
            chip8_isa_jmp(chip, opcode);
            break;

        case 0x2:
            chip8_isa_jsr(chip, opcode);
            break;

        case 0x3:
            chip8_isa_skeq_immediate(chip, opcode);
            break;

        case 0x4:
            chip8_isa_skne_immediate(chip, opcode);
            break;

        case 0x5:
            chip8_isa_skeq_reg(chip, opcode);
            break;

        case 0x6:
            chip8_isa_mov(chip, opcode, false);
            break;

        case 0x7:
            chip8_isa_add(chip, opcode, false);
            break;

        case 0x8: {
            switch (opcode & 0x000f) {
                case 0x0:
                    chip8_isa_mov(chip, opcode, true);
                    break;

                case 0x1:
                    chip8_isa_or(chip, opcode);
                    break;

                case 0x2:
                    chip8_isa_and(chip, opcode);
                    break;

                case 0x3:
                    chip8_isa_xor(chip, opcode);
                    break;

                case 0x4:
                    chip8_isa_add(chip, opcode, true);
                    break;

                case 0x5:
                    chip8_isa_sub(chip, opcode, false);
                    break;

                case 0x7:
                    chip8_isa_sub(chip, opcode, true);
                    break;
                    
                default:
                    matches = false;
            }
            
            break;
        }

        case 0x9:
            chip8_isa_skne_reg(chip, opcode);
            break;

        case 0xa:
            chip8_isa_mvi(chip, opcode);
            break;

        case 0xd:
            chip8_isa_draw(chip, opcode);
            break;

        default:
            matches = false;
    }
    
    if (!matches) {
        fprintf(stderr, "unknown instruction: %04x\n", opcode);
        exit(EXIT_FAILURE);
    }
    if (chip->on_cycle) chip->on_cycle(chip);
}

void chip8_destroy(struct Chip8 *chip) {
    free(chip->display);
}

// -- chip8 internals
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
