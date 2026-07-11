#ifndef CHIP_H
#define CHIP_H

#include "display.h"
#include "keypad.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

extern const uint16_t ROM_ADDRESS;
extern const uint16_t FONT_ADDRESS;

// -- chip8 definition
struct Chip8 {
    // 4KiB of memory (same as original)
    uint8_t mem[4096];
    
    // program counter register (approx.: 12-bit)
    uint16_t pc;
    
    // index register (approx.: 12-bit)
    uint16_t i;
    
    // general purpose registers (0..15, or V0..VF)
    uint8_t v[16];

    // stack
    uint16_t stack[16];

    // current stack index
    size_t stack_index;

    // delay timer (0-255)
    uint8_t dt;

    // sound timer (0-255)
    uint8_t st;
    
    // display
    struct Chip8Display *display;

    // keypad
    struct Chip8Keypad *keypad;

    // callback to call every cpu cycle
    void (*on_cycle)(struct Chip8*);
};

void chip8_init(struct Chip8 *chip);
void chip8_reset(struct Chip8 *chip);
void chip8_load_rom(struct Chip8 *chip, const uint8_t *rom, size_t rom_length);
void chip8_cycle(struct Chip8 *chip);
void chip8_update_timers(struct Chip8 *chip);
void chip8_destroy(struct Chip8 *chip);

#endif