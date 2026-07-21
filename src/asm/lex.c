#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "asm/lex.h"
#include "asm/arena.h"

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
}

static int hexvalue(char c) {
    if (isdigit(c)) return c - '0';
    return (tolower(c) - 'a') + 10;
}

Token lex_next(Lex *lex) {
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

        while (isxdigit(lex_peek(lex))) {
            // shift existing digits to the left (base 16) and add the new digit
            tk.seminfo.i = tk.seminfo.i * 16 + hexvalue(lex_advance(lex));
        }
    } else if (isdigit(lex_peek(lex))) {
        tk.type = TK_NUMBER;

        while (isdigit(lex_peek(lex))) {
            // shift existing digits to the left (base 10) and add the new digit
            tk.seminfo.i = tk.seminfo.i * 10 + (lex_advance(lex) - '0');
        }
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
            lex_skip(lex, 1); // skip \n
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

int main(void) {
    const char *src =
        "LOOP: LD vA, 0xFF\n"
        "  ADD v0, v1\n"
        "  SE v2, 10\n"
        "  JP LOOP.end\n"
        "; comment line\n\n\n";

    ArenaAllocator arena;
    arena_init(&arena);

    Lex lex;
    lex_init(&lex, &arena, src);

    Token tk;
    do {
        tk = lex_next(&lex);
        printf("[%zu:%zu] %-10s", tk.line, tk.column, lex_token2str(tk.type));

        switch (tk.type) {
            case TK_REGISTER:
                printf(" v%X", tk.seminfo.r);
                break;
            case TK_NUMBER:
                printf(" %u (0x%X)", tk.seminfo.i, tk.seminfo.i);
                break;
            case TK_IDENTIFIER:
                printf(" \"%s\"", tk.seminfo.id);
                break;
            default:
                break;
        }
        printf("\n");
    } while (tk.type != TK_EOS);

    printf("OK: lexer ran to completion\n");
    return 0;
}
