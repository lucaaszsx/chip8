#ifndef CHIP8_ISA_H
#define CHIP8_ISA_H

#include <stdint.h>
#include "chip8.h"

void chip8_isa_cls(struct Chip8 *chip);
void chip8_isa_rts(struct Chip8 *chip);
void chip8_isa_jmp(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_jsr(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_skeq_immediate(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_skne_immediate(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_skeq_reg(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_mov(struct Chip8 *chip, uint16_t opcode, bool is_reg);
void chip8_isa_add(struct Chip8 *chip, uint16_t opcode, bool is_reg);
void chip8_isa_sub(struct Chip8 *chip, uint16_t opcode, bool rsb);
void chip8_isa_or(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_and(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_xor(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_shr(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_shl(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_skne_reg(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_mvi(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_jmi(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_rand(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_draw(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_skpr(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_skup(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_gdelay(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_sdelay(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_ssound(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_adi(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_key(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_font(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_bcd(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_str(struct Chip8 *chip, uint16_t opcode);
void chip8_isa_ldr(struct Chip8 *chip, uint16_t opcode);

#endif