#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include "chip8.h"

#define APP_TITLE "Chip8 Emulator"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define PIXEL_WIDTH ((int)(SCREEN_WIDTH / DISPLAY_COLS))
#define PIXEL_HEIGHT ((int)(SCREEN_HEIGHT / DISPLAY_ROWS))

struct Chip8Renderer {
    SDL_Window *window;
    SDL_Renderer *renderer;

    struct Chip8 *chip;
};

void chip8_renderer_init(struct Chip8Renderer *rd, struct Chip8 *chip);
void chip8_renderer_render(struct Chip8Renderer *rd);
void chip8_renderer_destroy(struct Chip8Renderer *rd);

#endif