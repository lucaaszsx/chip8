#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

// -- display definition
struct Chip8Display {
    // vram (64x32 pixels)
    bool vram[2048];
};

void chip8_display_reset(struct Chip8Display *display);
void chip8_display_clear(struct Chip8Display *display);
uint8_t chip8_display_draw(struct Chip8Display *display, uint8_t x, uint8_t y, uint8_t n, uint8_t *sprite);

#endif