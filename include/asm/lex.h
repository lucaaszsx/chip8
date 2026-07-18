#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TK_IDENTIFIER,
    TK_INT_HEX,
    TK_INT,

    // Delimiters
    TK_SEMICOLON,
    TK_COLON,
    TK_COMMA,
    TK_DOT,

    // EOF
    TK_EOF
} TokenType;

struct Token {
    TokenType type;
    size_t line;
    size_t column;
    char *raw;
};

struct Lex {
    const char *src;
    size_t src_len;
    size_t line;
    size_t column;
    size_t pos;
};

void lex_init(struct Lex *lex);
struct Token *lex_next_token(struct Lex *lex);
char *token_type_name(const struct Token *tk);
