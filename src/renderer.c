#include "renderer.h"
#include <SDL3/SDL.h>
#include <stddef.h>
#include <math.h>

void render_chip8_display(SDL_Renderer *renderer, struct Chip8Display *display) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (uint16_t i = 0; i < sizeof(display->vram); i++) {
        if (display->vram[i] != 1) continue;
        SDL_FRect rect = {(i % 64) * 10, ceil(i / 64) * 10, 10, 10};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}
