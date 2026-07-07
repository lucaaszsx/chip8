#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

// -- display definition
struct Chip8Display {
    // screen (64x32 pixels)
    bool screen[64 * 32];
};

#endif