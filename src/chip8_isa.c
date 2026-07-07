#include "chip8_isa.h"
#include <stdint.h>
#include <stdio.h>

static uint8_t chip8_X(uint16_t opcode);
static uint8_t chip8_Y(uint16_t opcode);
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
uint8_t chip8_X(uint16_t opcode) {
    return (opcode & 0x0f00) >> 8;
}

uint8_t chip8_Y(uint16_t opcode) {
    return (opcode & 0x00f0) >> 4;
}

uint8_t chip8_NN(uint16_t opcode) {
    return opcode & 0x00ff;
}

uint16_t chip8_NNN(uint16_t opcode) {
    return opcode & 0x0fff;
}