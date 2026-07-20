#pragma once

#include <stdbool.h>
#include <stddef.h>

#define ARENA_DEFAULT_SIZE 1024

typedef struct {
    unsigned char *buffer;
    size_t capacity;
    size_t offset;
} ArenaAllocator;

bool arena_init(ArenaAllocator *arena);
void *arena_allocate(ArenaAllocator *arena, size_t count);
void *arena_destroy(ArenaAllocator *arena);
void *arena_strdup(ArenaAllocator *arena, const char *src, size_t len);
