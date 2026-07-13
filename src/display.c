#include <string.h>
#include "display.h"

void chip8_display_reset(struct Chip8Display *display) {
    memset(display->vram, 0, sizeof(display->vram));
}

uint8_t chip8_display_draw(struct Chip8Display *display, uint8_t x, uint8_t y, uint8_t n, uint8_t *sprite) {
    bool has_collision = false;

    for (size_t i = 0; i < n; i++) {
        uint8_t row = sprite[i];
        uint8_t py = (y + i) % DISPLAY_ROWS;

        for (int j = 0; j < 8; j++) {
            uint8_t px = (x + j) % DISPLAY_COLS;
            uint16_t index = (py * DISPLAY_COLS) + px;

            uint8_t sprite_bit = (row & (1 << (7 - j))) != 0;
            if (display->vram[index] && sprite_bit) has_collision = true;

            display->vram[index] ^= sprite_bit;
        }
    }

    return has_collision;
}