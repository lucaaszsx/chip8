#include <stdint.h>
#include "chip8.h"

int main() {
    struct Chip8 chip;
    chip8_init(&chip, 500);
    
    uint8_t program[] = {
        0x70, 0x01, // [0x200] 7XKK (VX = VX + KK) -> 7007 (V0 = V0 + 01H)
        0x12, 0x00, // [0x202] 1KKK (jmp KKK) -> 1200 (jmp 200H)
    };
    chip8_load_prog(&chip, program, sizeof(program) / sizeof(uint8_t));
    chip8_loop(&chip);
    
    return 0;
}