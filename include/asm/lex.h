#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "asm/arena.h"

typedef enum {
    TK_UNKNOWN = -1,
    TK_IDENTIFIER,
    TK_REGISTER,
    TK_NUMBER,
    TK_LABEL,
    TK_DIRECTIVE,

    // Delimiters
    TK_COLON,
    TK_COMMA,
    TK_DOT,

    TK_NEWLINE,

    // EOS - End Of Source
    TK_EOS
} TokenType;

/* semantic information (yes, Lua-style) */
typedef union {
    uint16_t i; /* integer value */
    char *id; /* identifier */
    uint8_t r; /* a register (0-15) */
} SemInfo;

typedef struct {
    TokenType type;
    size_t line;
    size_t column;
    SemInfo seminfo;
} Token;

typedef struct {
    const char *src;
    size_t src_len;

    size_t line; /* current lexer line */
    size_t column; /* current lexer line */
    size_t pos; /* current lexer position at src (<src_len) */

    Token lookahead;
    bool has_lookahead;

    ArenaAllocator *arena;
} Lex;

void lex_init(Lex *lex, ArenaAllocator *arena, const char *src);
Token lex_next(Lex *lex);
Token lex_lookahead(Lex *lex);
const char *lex_token2str(TokenType type);
