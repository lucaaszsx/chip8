#pragma once

#include <stdbool.h>
#include <stddef.h>

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
bool istrcasecmp(const char *s1, const char *s2);
