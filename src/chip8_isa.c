#include "chip8_isa.h"
#include <stdint.h>
#include <stdio.h>

static uint8_t chip8_X(uint16_t opcode);
static uint8_t chip8_Y(uint16_t opcode);
static uint8_t chip8_N(uint16_t opcode);
static uint8_t chip8_NN(uint16_t opcode);
static uint16_t chip8_NNN(uint16_t opcode);

void chip8_isa_jmp(struct Chip8 *chip, uint16_t opcode) {
    chip->pc = chip8_NNN(opcode);
}

void chip8_isa_mov(struct Chip8 *chip, uint16_t opcode) {
    chip->v[chip8_X(opcode)] = chip8_NN(opcode);
}

void chip8_isa_add(struct Chip8 *chip, uint16_t opcode) {
    uint8_t reg = chip8_X(opcode);
    chip->v[reg] = chip->v[reg] + chip8_NN(opcode);
}

void chip8_isa_mvi(struct Chip8 *chip, uint16_t opcode) {
    chip->i = chip8_NNN(opcode);
}

void chip8_isa_sprite(struct Chip8 *chip, uint16_t opcode) {
    printf("cannot draw sprites yet");
    exit(1);
}

// internals

/** 
 * Take the second part of the opcode hexadecimal value
 * Used to lookup for some register between V0..VF
 * e.g.: 0x610A -> 0x1 (1)
 */
uint8_t chip8_X(uint16_t opcode) {
    return (opcode & 0x0f00) >> 8;
}

/** 
 * Take the third part of the opcode hexadecimal value
 * Also used to lookup for registers
 */
uint8_t chip8_Y(uint16_t opcode) {
    return (opcode & 0x00f0) >> 4;
}

/** 
 * An 4 bit number formed by the fourth part of the opcode hexadecimal value
 * e.g.: 0x610A -> 0xA (10)
 */
uint8_t chip8_N(uint16_t opcode) {
    return opcode & 0x000f;
}

/** 
 * An 8 bit number formed by the third and fourth parts of the opcode hexadecimal value
 * e.g.: 0x601A -> 0x1A (26)
 */
uint8_t chip8_NN(uint16_t opcode) {
    return opcode & 0x00ff;
}


/** 
 * An 12 bit number formed by the second, third and fourth parts of the opcode hexadecimal value
 * e.g.: 0x1200 -> 0x200 (512)
 */
uint16_t chip8_NNN(uint16_t opcode) {
    return opcode & 0x0fff;
}