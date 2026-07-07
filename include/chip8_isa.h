#ifndef CHIP8_ISA_H
#define CHIP8_ISA_H

#include <stdint.h>
#include "chip8.h"

void chip8_isa_cls(struct Chip8 *chip);
void chip8_isa_rts(struct Chip8 *chip);
void chip8_isa_jmp(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_jsr(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_mov(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_add(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_mvi(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_sprite(struct Chip8 *chip, uint16_t opcode);

#endif