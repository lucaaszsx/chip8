#include <stdlib.h>
#include <stdio.h>
#include "emu/display.h"
#include "emu/memory.h"
#include "emu/keypad.h"
#include "emu/chip8.h"
#include "emu/isa.h"

static uint8_t chip8_X(uint16_t opcode);
static uint8_t chip8_Y(uint16_t opcode);
static uint8_t chip8_N(uint16_t opcode);
static uint8_t chip8_NN(uint16_t opcode);
static uint16_t chip8_NNN(uint16_t opcode);

void chip8_isa_cls(struct Chip8 *chip) {
    chip8_display_reset(chip->display);
    chip->draw_flag = true;
}

void chip8_isa_rts(struct Chip8 *chip) {
    size_t index = --chip->stack_index;
    chip->pc = chip->stack[index];
    chip->stack[index] = 0;
}

void chip8_isa_jmp(struct Chip8 *chip, uint16_t opcode) {
    chip->pc = chip8_NNN(opcode);
}

void chip8_isa_jsr(struct Chip8 *chip, uint16_t opcode) {
    chip->stack[chip->stack_index++] = chip->pc;
    chip->pc = chip8_NNN(opcode);
}

void chip8_isa_skeq_immediate(struct Chip8 *chip, uint16_t opcode) {
    if (chip->v[chip8_X(opcode)] == chip8_NN(opcode))
        chip->pc += 2;
}

void chip8_isa_skne_immediate(struct Chip8 *chip, uint16_t opcode) {
    if (chip->v[chip8_X(opcode)] != chip8_NN(opcode))
        chip->pc += 2;
}

void chip8_isa_skeq_reg(struct Chip8 *chip, uint16_t opcode) {
    if (chip->v[chip8_X(opcode)] == chip->v[chip8_Y(opcode)])
        chip->pc += 2;
}

void chip8_isa_mov(struct Chip8 *chip, uint16_t opcode, bool is_reg) {
    uint8_t reg_x = chip8_X(opcode);
    
    if (is_reg) chip->v[reg_x] = chip->v[chip8_Y(opcode)];
    else chip->v[reg_x] = chip8_NN(opcode);
}

void chip8_isa_add(struct Chip8 *chip, uint16_t opcode, bool is_reg) {
    uint8_t reg_x = chip8_X(opcode);
    uint8_t x = chip->v[reg_x];

    if (is_reg) {
        uint8_t y = chip->v[chip8_Y(opcode)];
        int16_t result = (int16_t)x + (int16_t)y;

        chip->v[reg_x] = result;
        chip->v[0xf] = result > UINT8_MAX ? 1 : 0;
    } else chip->v[reg_x] += chip8_NN(opcode);
}

void chip8_isa_sub(struct Chip8 *chip, uint16_t opcode, bool rsb) {
    uint8_t reg_x = chip8_X(opcode);
    uint8_t reg_y = chip8_Y(opcode);
    
    uint8_t x = chip->v[reg_x];
    uint8_t y = chip->v[reg_y];

    if (rsb) {
        chip->v[reg_x] = y - x;
        chip->v[0xf] = y >= x ? 1 : 0;
    } else {
        chip->v[reg_x] = x - y;
        chip->v[0xf] = x >= y ? 1 : 0;
    }
}

void chip8_isa_or(struct Chip8 *chip, uint16_t opcode) {
    chip->v[chip8_X(opcode)] |= chip->v[chip8_Y(opcode)];
}

void chip8_isa_and(struct Chip8 *chip, uint16_t opcode) {
    chip->v[chip8_X(opcode)] &= chip->v[chip8_Y(opcode)];
}

void chip8_isa_xor(struct Chip8 *chip, uint16_t opcode) {
    chip->v[chip8_X(opcode)] ^= chip->v[chip8_Y(opcode)];
}

void chip8_isa_shr(struct Chip8 *chip, uint16_t opcode) {
    uint8_t reg_x = chip8_X(opcode);
    uint8_t x = chip->v[reg_x];

    chip->v[reg_x] >>= 1;
    chip->v[0xf] = x & 0x1;
}

void chip8_isa_shl(struct Chip8 *chip, uint16_t opcode) {
    uint8_t reg_x = chip8_X(opcode);
    uint8_t x = chip->v[reg_x];

    chip->v[reg_x] <<= 1;
    chip->v[0xf] = (x >> 7) & 0x1;
}

void chip8_isa_skne_reg(struct Chip8 *chip, uint16_t opcode) {
    if (chip->v[chip8_X(opcode)] != chip->v[chip8_Y(opcode)])
        chip->pc += 2;
}

void chip8_isa_mvi(struct Chip8 *chip, uint16_t opcode) {
    chip->i = chip8_NNN(opcode);
}

void chip8_isa_jmi(struct Chip8 *chip, uint16_t opcode) {
    chip->pc = chip8_NN(opcode) + chip->v[chip8_X(opcode)];
}

void chip8_isa_rand(struct Chip8 *chip, uint16_t opcode) {
    chip->v[chip8_X(opcode)] = rand() & chip8_NN(opcode);
}

void chip8_isa_draw(struct Chip8 *chip, uint16_t opcode) {
    uint8_t x = chip->v[chip8_X(opcode)];
    uint8_t y = chip->v[chip8_Y(opcode)];
    uint8_t n = chip8_N(opcode);
    uint8_t sprite[n];
    
    for (uint8_t segment = 0; segment < n; segment++){
        sprite[segment] = chip8_mem_read(chip, chip->i + segment);
    }
    
    chip->v[0xf] = chip8_display_draw(chip->display, x, y, n, sprite);
    chip->draw_flag = true;
}

void chip8_isa_skpr(struct Chip8 *chip, uint16_t opcode) {
    if (chip8_keypad_pressed(chip->keypad, chip->v[chip8_X(opcode)]))
        chip->pc += 2;
}

void chip8_isa_skup(struct Chip8 *chip, uint16_t opcode) {
    if (!chip8_keypad_pressed(chip->keypad, chip->v[chip8_X(opcode)]))
        chip->pc += 2;
}

void chip8_isa_gdelay(struct Chip8 *chip, uint16_t opcode) {
    chip->v[chip8_X(opcode)] = chip->dt;
}

void chip8_isa_sdelay(struct Chip8 *chip, uint16_t opcode) {
    chip->dt = chip->v[chip8_X(opcode)];
}

void chip8_isa_ssound(struct Chip8 *chip, uint16_t opcode) {
    chip->st = chip->v[chip8_X(opcode)];
}

void chip8_isa_adi(struct Chip8 *chip, uint16_t opcode) {
    uint16_t result = chip->i + chip->v[chip8_X(opcode)];

    chip->v[0xf] = (result > UINT8_MAX) ? 1 : 0;
    chip->i = result;
}

void chip8_isa_key(struct Chip8 *chip, uint16_t opcode) {
    if (!chip->keypad->waiting)
        chip8_keypad_wait(chip->keypad);

    uint8_t key;
    if (chip8_keypad_poll(chip->keypad, &key))
        chip->v[chip8_X(opcode)] = key;
    else
        chip->pc -= 2;
}

void chip8_isa_font(struct Chip8 *chip, uint16_t opcode) {
    uint8_t vx = chip->v[chip8_X(opcode)];
    chip->i = FONT_ADDRESS + (vx & 0x00ff) * 5;
}

void chip8_isa_bcd(struct Chip8 *chip, uint16_t opcode) {
    uint8_t value = chip->v[chip8_X(opcode)];
    uint8_t digits[3] = {
        value / 100,
        (value / 10) % 10,
        value % 10
    };
    
    chip8_mem_write_many(chip, chip->i, digits, 3);
}

void chip8_isa_str(struct Chip8 *chip, uint16_t opcode) {
    chip8_mem_write_many(chip, chip->i, chip->v, chip8_X(opcode) + 1); // v0..vx
}

void chip8_isa_ldr(struct Chip8 *chip, uint16_t opcode) {
    for (size_t i = 0; i <= chip8_X(opcode); i++)
        chip->v[i] = chip8_mem_read(chip, chip->i + i);
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
