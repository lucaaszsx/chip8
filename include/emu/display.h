#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_ROWS 32
#define DISPLAY_COLS 64
#define DISPLAY_SIZE (DISPLAY_ROWS * DISPLAY_COLS)

// -- display definition
struct Chip8Display {
    // vram (64x32 pixels)
    bool vram[DISPLAY_SIZE];
};

void chip8_display_reset(struct Chip8Display *display);
uint8_t chip8_display_draw(struct Chip8Display *display, uint8_t x, uint8_t y, uint8_t n, uint8_t *sprite);
