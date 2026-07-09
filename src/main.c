#include <stdint.h>
#include <stdio.h>
#include "chip8.h"

static void on_cycle(struct Chip8 *chip) {
    static int first_call = 1;
    static char buf[8192];
    size_t len = 0;

    if (first_call) {
        len += snprintf(buf + len, sizeof(buf) - len, "\033[2J");
        first_call = 0;
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\033[H");
    len += snprintf(buf + len, sizeof(buf) - len, "---------- Chip-8 ----------\n");
    len += snprintf(buf + len, sizeof(buf) - len, "REGISTERS:\n");
    len += snprintf(buf + len, sizeof(buf) - len, "  PC (program counter): 0x%x\n", chip->pc);
    len += snprintf(buf + len, sizeof(buf) - len, "  Byte at 0x%x: 0x%x\n", chip->pc, chip->mem[chip->pc]);
    len += snprintf(buf + len, sizeof(buf) - len, "  Byte at 0x%x+1: 0x%x\n", chip->pc, chip->mem[chip->pc + 1]);
    len += snprintf(buf + len, sizeof(buf) - len, "  I (index): 0x%x\n", chip->i);
    len += snprintf(buf + len, sizeof(buf) - len, "  General Purpose Registers (GPR):\n");

    for (size_t i = 0; i < sizeof(chip->v); i++) {
        if (i % 8 == 0) {
            if (i > 0) len += snprintf(buf + len, sizeof(buf) - len, "\n");
            len += snprintf(buf + len, sizeof(buf) - len, "    ");
        }
        len += snprintf(buf + len, sizeof(buf) - len, "V%02zu=0x%02x    ", i, chip->v[i]);
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\n");

    len += snprintf(buf + len, sizeof(buf) - len, "MEMORY (512..768):\n");
    for (size_t i = 512; i < 768; i++) {
        if (i % 32 == 0) {
            if (i > 512) len += snprintf(buf + len, sizeof(buf) - len, "\n");
            len += snprintf(buf + len, sizeof(buf) - len, "  ");
        }
        len += snprintf(buf + len, sizeof(buf) - len, i == chip->pc ? "\033[31;1;4m%02x\033[0m " : "%02x ", chip->mem[i]);
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\n\033[J");

    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}

int main() {
    struct Chip8 chip;
    chip8_init(&chip, 5);
    
    chip.on_cycle = on_cycle;
    
    uint8_t program[] = {
        0x00, 0xe0, // [0x200] 00E0 (cls)
        0xa2, 0x2a, // [0x202] ANNN (mvi I, 22AH)
        0x60, 0x0c, // [0x204] 6XKK (mov V0, 0CH)
        0x61, 0x08, // [0x206] 6XKK (mov V1, 08H)
        0xd0, 0x1f, // [0x208] DXYN (sprite V0, V1, 0FH)
        0x70, 0x09, // [0x20A] 7XKK (add V0, 09H)
        0xa2, 0x39, // [0x20C] ANNN (mvi I, 239H)
        0xd0, 0x1f, // [0x20E] DXYN (sprite V0, V1, 0FH)
        0xa2, 0x48, // [0x210] ANNN (mvi I, 248H)
        0x70, 0x08, // [0x212] 7XKK (add V0, 08H)
        0xd0, 0x1f, // [0x214] DXYN (sprite V0, V1, 0FH)
        0x70, 0x04, // [0x216] 7XKK (add V0, 04H)
        0xa2, 0x57, // [0x218] ANNN (mvi I, 257H)
        0xd0, 0x1f, // [0x21A] DXYN (sprite V0, V1, 0FH)
        0x70, 0x08, // [0x21C] 7XKK (add V0, 08H)
        0xa2, 0x66, // [0x21E] ANNN (mvi I, 266H)
        0xd0, 0x1f, // [0x220] DXYN (sprite V0, V1, 0FH)
        0x70, 0x08, // [0x222] 7XKK (add V0, 08H)
        0xa2, 0x75, // [0x224] ANNN (mvi I, 275H)
        0xd0, 0x1f, // [0x226] DXYN (sprite V0, V1, 0FH)
        0x12, 0x28, // [0x228] 1NNN (jmp 228H)

        // [0x22A] Sprite data
        0xff, 0x00,
        0xff, 0x00,
        0x3c, 0x00,
        0x3c, 0x00,
        0x3c, 0x00,
        0x3c, 0x00,
        0xff, 0x00,
        0xff, 0xff,
        0x00, 0xff,
        0x00, 0x38,
        0x00, 0x3f,
        0x00, 0x3f,
        0x00, 0x38,
        0x00, 0xff,
        0x00, 0xff,
        0x80, 0x00,
        0xe0, 0x00,
        0xe0, 0x00,
        0x80, 0x00,
        0x80, 0x00,
        0xe0, 0x00,
        0xe0, 0x00,
        0x80, 0xf8,
        0x00, 0xfc,
        0x00, 0x3e,
        0x00, 0x3f,
        0x00, 0x3b,
        0x00, 0x39,
        0x00, 0xf8,
        0x00, 0xf8,
        0x03, 0x00,
        0x07, 0x00,
        0x0f, 0x00,
        0xbf, 0x00,
        0xfb, 0x00,
        0xf3, 0x00,
        0xe3, 0x00,
        0x43, 0xe0,
        0x00, 0xe0,
        0x00, 0x80,
        0x00, 0x80,
        0x00, 0x80,
        0x00, 0x80,
        0x00, 0xe0,
        0x00, 0xe0,
    };
    chip8_load_prog(&chip, program, sizeof(program) / sizeof(uint8_t));
    chip8_loop(&chip);
    
    return 0;
}

/**
 * asm code:
 * mov v0, 0x1
 * skeq v0, 10
 * jsr 0x20a
 * mov v1, v0
 * jmp 0x202
 * add v0, 0x1
 * rts
 */