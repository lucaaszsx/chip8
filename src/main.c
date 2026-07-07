#include <stdint.h>
#include "chip8.h"

int main() {
    struct Chip8 chip;
    chip8_init(&chip, 10);
    
    uint8_t program[] = {
        0x60, 0x01, // [0x200] 6XKK (mov VX, KK) -> 6001 (mov V0, 0x01)
        0x30, 0x0a, // [0x202] 3XKK (skeq VX, KK) -> 300a (skeq V0, 0x0a)
        0x22, 0x08, // [0x204] 2KKK (jsr KKK) -> 2208 (jsr 0x208)
        0x12, 0x02, // [0x206] 1KKK (jmp KKK) -> 1202 (jmp 0x202)
        0x70, 0x01, // [0x208] 7XKK (add VX, KK) -> 7001 (add V0, 0x01)
        0x00, 0xee, // [0x20a] 00ee -> rts
    };
    chip8_load_prog(&chip, program, sizeof(program) / sizeof(uint8_t));
    chip8_loop(&chip);
    
    return 0;
}

/**
 * asm code:
 * mov v0, 0x1
 * skeq v0, 10
 * jsr 0x208
 * jmp 0x202
 * add v0, 0x1
 * rts
 */