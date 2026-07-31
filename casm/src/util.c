#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (p == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

int istrcasecmp(const char *s1, const char *s2) {
    while (*s1 && (tolower((unsigned char)*s1) == tolower((unsigned char)*s2))) {
        s1++;
        s2++;
    }

    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}
