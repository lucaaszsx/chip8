#include <math.h>
#include <stdlib.h>
#include "renderer.h"
#include "display.h"

void chip8_renderer_init(struct Chip8Renderer *rd, struct Chip8 *chip) {
    SDL_Window *window;
    if ((window = SDL_CreateWindow(APP_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT, 0)) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL window initialization failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer *renderer;
    if ((renderer = SDL_CreateRenderer(window, NULL)) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL renderer initialization failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    rd->window = window;
    rd->renderer = renderer;
    rd->chip = chip;
}

void chip8_renderer_render(struct Chip8Renderer *rd) {
    SDL_SetRenderDrawColor(rd->renderer, 0, 0, 0, 255);
    SDL_RenderClear(rd->renderer);

    SDL_SetRenderDrawColor(rd->renderer, 255, 255, 255, 255);
    
    for (size_t i = 0; i < DISPLAY_SIZE; i++) {
        if (rd->chip->display->vram[i] != 1) continue;
        
        SDL_FRect rect = {
            .x=(i % DISPLAY_COLS) * PIXEL_WIDTH,
            .y=(int)(i / DISPLAY_COLS) * PIXEL_HEIGHT,
            .w=PIXEL_WIDTH,
            .h=PIXEL_HEIGHT
        };
        SDL_RenderFillRect(rd->renderer, &rect);
    }

    SDL_RenderPresent(rd->renderer);
}

void chip8_renderer_destroy(struct Chip8Renderer *rd) {
    SDL_DestroyWindow(rd->window);
    rd->window = NULL;
    
    SDL_DestroyRenderer(rd->renderer);
    rd->renderer = NULL;
    
    SDL_Quit();
}
