#pragma once

#include <stdint.h>
#include "chip8.h"

void chip8_mem_write(struct Chip8 *chip, uint16_t addr, uint8_t word);
void chip8_mem_write_many(struct Chip8 *chip, uint16_t start_addr, const uint8_t *words, size_t length);
uint8_t chip8_mem_read(const struct Chip8 *chip, uint16_t addr);
