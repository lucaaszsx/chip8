#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} ByteBuffer;

void buf_init(ByteBuffer *buf, size_t icap);
void buf_free(ByteBuffer *buf);
void buf_write_u8(ByteBuffer *buf, uint8_t byte);
void buf_write_u16(ByteBuffer *buf, uint16_t value);
