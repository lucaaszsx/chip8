#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include "display.h"

void render_chip8_display(SDL_Renderer *renderer, struct Chip8Display *display);

#endif