#include <stdint.h>
#include <stdio.h>
#include "chip8.h"

void print_chip(const struct Chip8 *chip, char *label) {
    printf("\n---------- %s ----------\n", label);

    printf("Chip8 PC reg: 0x%x\n", chip->pc);
    printf("Chip8 I reg: 0x%x\n", chip->i);

    printf("General purpose registers:\n");
    for (size_t i = 0; i < 16; i++) {
        printf("  V%d: 0x%x\n", i, chip->v[i]);
    }
    printf("\n");
}

void print_mem(struct Chip8 *chip, size_t start_offset, size_t end_offset, size_t per_line) {
    for (uint16_t addr = start_offset; addr < start_offset + end_offset; addr++) {
        if ((addr - start_offset) % per_line == 0) printf("\n");
        printf("%X ", chip->mem[addr]);
    }
    printf("\n");
}

int main() {
    struct Chip8 chip;
    chip8_init(&chip);
    print_chip(&chip, "Chip started");
    
    uint8_t program[] = {
        0x60, 0x06, // 6XKK (VX = KK) -> 6006 (V0 = 06H)
        0x70, 0x07, // 7XKK (VX = VX + KK) -> 7007 (V0 = V0 + 07H)
    };
    chip8_load_prog(&chip, program, sizeof(program) / sizeof(uint8_t));
    print_mem(&chip, 512, 200, 10);
    chip8_loop(&chip);
    print_chip(&chip, "Chip after loop");
    
    return 0;
}