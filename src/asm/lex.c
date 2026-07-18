#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "asm/lex.h"

static char lex_advance(struct Lex *lex);
static void lex_skip(struct Lex *lex, size_t n);
static char lex_peek(struct Lex *lex);
static char lex_lookahead(struct Lex *lex);
static void lex_skip_trivia(struct Lex *lex);
struct Token *lex_next_token(struct Lex *lex);
static void lex_set_tk_raw(struct Lex *lex, struct Token *tk, TokenType type, size_t start, size_t end);
static bool lex_eof(struct Lex *lex);
static bool is_hex_digit(char c);
static bool is_delimiter(char c);
static TokenType get_delimiter_type(char c);

static const char *source =
    ".org 0x200\n"
    "start:\n"
    "   jmp start\n";

void lex_init(struct Lex *lex) {
    lex->src = source;
    lex->src_len = strlen(lex->src);
    lex->line = 1;
    lex->column = 1;
    lex->pos = 0;
}

static char lex_advance(struct Lex *lex) {
    if (lex_eof(lex)) return '\0';

    char c = lex->src[lex->pos++];
    if (c == '\n') {
        lex->column = 0;
        lex->line++;
    } else lex->column++;

    return c;
}

static void lex_skip(struct Lex *lex, size_t n) {
    lex->pos += n;
}

static char lex_peek(struct Lex *lex) {
    return lex->src[lex->pos];
}

static char lex_lookahead(struct Lex *lex) {
    if (lex_eof(lex)) return '\0';
    return lex->src[lex->pos + 1];
}

static void lex_skip_trivia(struct Lex *lex) {
    while (true) {
        if (isspace(lex_peek(lex))) lex_advance(lex);
        else if (lex_peek(lex) == ';') {
            while (!lex_eof(lex) && lex_peek(lex) != '\n')
                lex_advance(lex);
        } else break;
    }
}

struct Token *lex_next_token(struct Lex *lex) {
    lex_skip_trivia(lex);

    struct Token *token = malloc(sizeof(struct Token));
    if (token == NULL) {
        fprintf(stderr, "memory allocation for the token failed");
        exit(EXIT_FAILURE);
    }

    token->line = lex->line;
    token->column = lex->column;

    if (lex_peek(lex) == '\0') {
        lex_set_tk_raw(lex, token, TK_EOF, lex->pos, lex->pos);
        return token;
    }

    if (isalpha(lex_peek(lex)) || lex_peek(lex) == '_') {
        size_t start = lex->pos;

        while (isalnum(lex_peek(lex)) || lex_peek(lex) == '_') lex_advance(lex);
        lex_set_tk_raw(lex, token, TK_IDENTIFIER, start, lex->pos);
    } else if (lex_peek(lex) == '0' && tolower(lex_lookahead(lex)) == 'x') { // 0x
        lex_skip(lex, 2); // skip "0x"

        size_t start = lex->pos;

        while (is_hex_digit(lex_peek(lex))) lex_advance(lex);
        lex_set_tk_raw(lex, token, TK_INT_HEX, start, lex->pos);
    } else if (isdigit(lex_peek(lex))) {
        size_t start = lex->pos;

        while (isdigit(lex_peek(lex))) lex_advance(lex);
        lex_set_tk_raw(lex, token, TK_INT, start, lex->pos);
    } else if (is_delimiter(lex_peek(lex))) {
        lex_set_tk_raw(lex, token, get_delimiter_type(lex_advance(lex)), lex->pos, lex->pos + 1);
    } else {
        fprintf(stderr, "invalid or unexpected token at %zu:%zu: %c\n", lex->line, lex->column, lex_peek(lex));
        exit(EXIT_FAILURE);
    }

    return token;
}

static void lex_set_tk_raw(struct Lex *lex, struct Token *tk, TokenType type, size_t start, size_t end) {
    size_t len = end - start;

    tk->raw = malloc(len + 1);
    memcpy(tk->raw, lex->src + start, len);
    tk->raw[len] = '\0';

    tk->type = type;
}

static bool is_hex_digit(char c) {
    return (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f') ||
            isdigit(c);
}

static bool is_delimiter(char c) {
    switch (c) {
        case ';':
        case ':':
        case ',':
        case '.':
            return true;

        default:
            return false;
    }
}

static bool lex_eof(struct Lex *lex) {
    return lex_peek(lex) == '\0';
}

static TokenType get_delimiter_type(char c) {
    if (!is_delimiter(c)) {
        fprintf(stderr, "invalid delimiter: %c", c);
        exit(EXIT_FAILURE);
    }

    switch (c) {
        case ';':
            return TK_SEMICOLON;

        case ':':
            return TK_COLON;

        case ',':
            return TK_COMMA;

        case '.':
            return TK_DOT;

        default:
            fprintf(stderr, "unknown delimiter: %c", c);
            exit(EXIT_FAILURE);
    }
}

static char *token_type_name(const struct Token *tk) {
    switch (tk->type) {
        case TK_IDENTIFIER: return "IDENTIFIER";
        case TK_INT_HEX: return "INT_HEX";
        case TK_INT: return "INT";

        case TK_SEMICOLON: return "SEMICOLON";
        case TK_COLON: return "COLON";
        case TK_COMMA: return "COMMA";
        case TK_DOT: return "DOT";

        case TK_EOF: return "EOF";

        default:
            fprintf(stderr, "unexpected token type: %d", tk->type);
            exit(EXIT_FAILURE);
    }
}
