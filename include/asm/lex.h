#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TK_IDENTIFIER,
    TK_NUMBER,

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
    uint16_t value;
};

struct Lex {
    const char *src;
    size_t src_len;
    size_t line;
    size_t column;
    size_t pos;
    struct Token *current;
};

void lex_init(struct Lex *lex);
void *lex_next_token(struct Lex *lex);
void *lex_expect(struct Lex *lex, TokenType expected);
char *token_type_name(TokenType type);
