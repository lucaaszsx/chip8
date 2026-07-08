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
        len += snprintf(buf + len, sizeof(buf) - len, "%02x ", chip->mem[i]);
    }
    len += snprintf(buf + len, sizeof(buf) - len, "\n\033[J");

    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}

int main() {
    struct Chip8 chip;
    chip8_init(&chip, 1000);
    
    chip.on_cycle = on_cycle;
    
    uint8_t program[] = {
        0x60, 0x01, // [0x200] 6XKK (mov VX, KK) -> 6001 (mov V0, 0x01)
        0x30, 0x2a, // [0x202] 3XKK (skeq VX, KK) -> 307f (skeq V0, 0xff)
        0x22, 0x0a, // [0x204] 2KKK (jsr KKK) -> 220a (jsr 0x20a)
        0x81, 0x00, // [0x206] 8XY0 (mov VX, VY) -> 8100 (mov V1, V0)
        0x12, 0x02, // [0x208] 1KKK (jmp KKK) -> 1202 (jmp 0x202)
        0x70, 0x01, // [0x20a] 7XKK (add VX, KK) -> 7001 (add V0, 0x01)
        0x00, 0xee, // [0x20c] 00ee -> rts
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