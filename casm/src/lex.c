#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include "lex.h"
#include "arena.h"

static char lex_advance(Lex *lex);
static char lex_skip(Lex *lex, size_t n);
static char lex_peek(Lex *lex);
static char lex_peeknext(Lex *lex);
static void lex_skip_trivia(Lex *lex);
static bool eos(Lex *lex);
static bool is_delimiter(char c);

void lex_init(Lex *lex, ArenaAllocator *arena, const char *src) {
    lex->arena = arena;
    lex->src = src;
    lex->src_len = strlen(lex->src);
    lex->line = 1;
    lex->column = 1;
    lex->pos = 0;
    lex->has_lookahead = false;
}

static int hexvalue(char c) {
    if (isdigit(c)) return c - '0';
    return (tolower(c) - 'a') + 10;
}

static uint16_t lex_read_digits(Lex *lex, int base, const Token *tk) {
    assert(base == 10 || base == 16);

    uint result = 0;
    while (
        (base == 10 && isdigit(lex_peek(lex))) ||
        (base == 16 && isxdigit(lex_peek(lex)))
    ) {
        char digit = lex_advance(lex);
        result = result * base + (base == 16 ? hexvalue(digit) : digit - '0');

        if (result > UINT16_MAX) {
            fprintf(stderr, "invalid number at %zu:%zu\n", tk->line, tk->column);
            exit(EXIT_FAILURE);
        }
    }

    return result;
}

static Token lex_jump(Lex *lex) {
    lex_skip_trivia(lex);

    Token tk;
    tk.line = lex->line;
    tk.column = lex->column;
    tk.seminfo.i = 0;
    tk.seminfo.id = NULL;
    tk.seminfo.r = 0;

    if (lex_peek(lex) == '\0') {
        tk.type = TK_EOS;
    } else if (lex_peek(lex) == '\n') {
        lex_skip(lex, 1);
        tk.type = TK_NEWLINE;
    } else if (tolower(lex_peek(lex)) == 'v') {
        lex_skip(lex, 1); // skip "v"

        if (!isxdigit((unsigned char)lex_peek(lex))) {
            fprintf(stderr, "invalid register at %zu:%zu: v%c\n", tk.line, tk.column, lex_peek(lex));
            exit(EXIT_FAILURE);
        }

        tk.type = TK_REGISTER;
        tk.seminfo.r = hexvalue(lex_advance(lex));
    } else if (isalpha(lex_peek(lex)) || lex_peek(lex) == '_') {
        tk.type = TK_IDENTIFIER;

        size_t start = lex->pos;
        while (isalnum(lex_peek(lex)) || lex_peek(lex) == '_') {
            lex_advance(lex);
        }

        tk.seminfo.id = arena_strdup(lex->arena, lex->src + start, lex->pos - start);
    } else if (lex_peek(lex) == '0' && tolower(lex_peeknext(lex)) == 'x') { // 0x
        lex_skip(lex, 2); // skip "0x"

        tk.type = TK_NUMBER;
        tk.seminfo.i = lex_read_digits(lex, 16, &tk);
    } else if (isdigit(lex_peek(lex))) {
        tk.type = TK_NUMBER;
        tk.seminfo.i = lex_read_digits(lex, 10, &tk);
    } else if (is_delimiter(lex_peek(lex))) {
        char c = lex_advance(lex);

        switch (c) {
            case ':':
                tk.type = TK_COLON;
                break;

            case ',':
                tk.type = TK_COMMA;
                break;

            case '.':
                tk.type = TK_DOT;
                break;

            default:
                fprintf(stderr, "unknown delimiter at %zu:%zu: %c\n", tk.line, tk.column, c);
                exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "invalid or unexpected token at %zu:%zu: %c\n", tk.line, tk.column, lex_peek(lex));
        exit(EXIT_FAILURE);
    }

    return tk;
}

Token lex_next(Lex *lex) {
    if (lex->has_lookahead) {
        lex->has_lookahead = false;
        return lex->lookahead;
    }

    return lex_jump(lex);
}

Token lex_lookahead(Lex *lex) {
    if (!lex->has_lookahead) {
        lex->has_lookahead = true;
        lex->lookahead = lex_jump(lex);
    }

    return lex->lookahead;
}

const char *lex_token2str(TokenType type) {
    switch (type) {
        case TK_IDENTIFIER: return "identifier";
        case TK_REGISTER: return "register";
        case TK_NUMBER:return "number";
        case TK_COLON: return "':'";
        case TK_COMMA: return "','";
        case TK_DOT: return "'.'";
        case TK_NEWLINE: return "<newline>";
        case TK_EOS: return "<eos>";

        case TK_UNKNOWN:
        default:
            return "unknown";
    }
}

static char lex_advance(Lex *lex) {
    if (eos(lex)) return '\0';

    char c = lex->src[lex->pos++];
    if (c == '\n') {
        lex->column = 0;
        lex->line++;
    } else lex->column++;

    return c;
}

static char lex_skip(Lex *lex, size_t n) {
    char c = lex_peek(lex);

    for (size_t j = 0; j < n; j++) lex_advance(lex);

    return c;
}

static char lex_peek(Lex *lex) {
    return lex->src[lex->pos];
}

static char lex_peeknext(Lex *lex) {
    if (eos(lex)) return '\0';
    return lex->src[lex->pos + 1];
}

static bool is_nonnewline_space(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

static void lex_skip_trivia(Lex *lex) {
    while (true) {
        if (is_nonnewline_space(lex_peek(lex))) lex_skip(lex, 1);
        else if (lex_peek(lex) == ';') { // skip single-line comments
            while (!eos(lex) && lex_peek(lex) != '\n')
                lex_skip(lex, 1);
        } else break;
    }
}

static bool eos(Lex *lex) {
    return lex_peek(lex) == '\0';
}

static bool is_delimiter(char c) {
    switch (c) {
        case ':':
        case ',':
        case '.':
            return true;

        default:
            return false;
    }
}

