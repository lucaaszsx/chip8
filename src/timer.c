#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "timer.h"

uint64_t get_ticks(void) {
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0) {
        perror("clock_gettime failed");
        exit(1);
    }

    return tp.tv_sec * 1000000000 + tp.tv_nsec;
}
