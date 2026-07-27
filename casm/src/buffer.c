#include <stdlib.h>
#include "buffer.h"
#include "util.h"

void buf_init(ByteBuffer *buf, size_t icap) {
    buf->data = xmalloc(icap * sizeof(uint8_t));
    buf->capacity = icap;
    buf->size = 0;
}

void buf_free(ByteBuffer *buf) {
    free(buf->data);

    buf->data = NULL;
    buf->capacity = 0;
    buf->size = 0;
}

void buf_write_u8(ByteBuffer *buf, uint8_t byte) {
    if (buf->size == buf->capacity) {
        buf->capacity *= 2;
        buf->data = xrealloc(buf->data, buf->capacity * sizeof(uint8_t));
    }

    buf->data[buf->size++] = byte;
}

void buf_write_u16(ByteBuffer *buf, uint16_t value) {
    buf_write_u8(buf, value >> 8);
    buf_write_u8(buf, value & 0xff);
}
