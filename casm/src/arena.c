#include <string.h>
#include <stdlib.h>
#include "arena.h"
#include "util.h"

void arena_init(ArenaAllocator *arena) {
    arena->buffer = xmalloc(ARENA_DEFAULT_SIZE);
    arena->capacity = ARENA_DEFAULT_SIZE;
    arena->offset = 0;
}

void arena_free(ArenaAllocator *arena) {
    free(arena->buffer);
}

void *arena_allocate(ArenaAllocator *arena, size_t count) {
    size_t end_offset = arena->offset + count;
    if (end_offset > arena->capacity) {
        size_t new_capacity = arena->capacity * 2;
        while (new_capacity < end_offset)
            new_capacity *= 2;

        arena->buffer = xrealloc(arena->buffer, new_capacity);
        arena->capacity = new_capacity;
    }

    void *ptr = arena->buffer + arena->offset;
    arena->offset += count;

    return ptr;
}

char *arena_strdup(ArenaAllocator *arena, const char *src, size_t len) {
    char *dst = arena_allocate(arena, len + 1);
    memcpy(dst, src, len);
    dst[len] = '\0';

    return dst;
}

uint8_t *arena_memcpy(ArenaAllocator *arena, const uint8_t *bytes, size_t count) {
    uint8_t *dst = arena_allocate(arena, count);
    memcpy(dst, bytes, count);

    return dst;
}
