#include <string.h>
#include <stdlib.h>
#include "asm/arena.h"

bool arena_init(ArenaAllocator *arena) {
    arena->buffer = malloc(ARENA_DEFAULT_SIZE);
    if (arena->buffer == NULL) return false;

    arena->capacity = ARENA_DEFAULT_SIZE;
    arena->offset = 0;

    return true;
}

void *arena_allocate(ArenaAllocator *arena, size_t count) {
    size_t end_offset = arena->offset + count;
    if (end_offset > arena->capacity) {
        size_t new_capacity = arena->capacity * 2;
        while (new_capacity < end_offset)
            new_capacity *= 2;

        void *new_buffer = realloc(arena->buffer, new_capacity);
        if (new_buffer == NULL) return NULL;

        arena->buffer = new_buffer;
        arena->capacity = new_capacity;
    }

    void *ptr = arena->buffer + arena->offset;
    arena->offset += count;

    return ptr;
}

char *arena_strdup(ArenaAllocator *arena, const char *src, size_t len) {
    char *dst = arena_allocate(arena, len + 1);
    if (dst == NULL) return NULL;

    memcpy(dst, src, len);
    dst[len] = '\0';

    return dst;
}
