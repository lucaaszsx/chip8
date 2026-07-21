#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "asm/parser.h"
#include "asm/lex.h"
#include "util.h"

static const struct {
    char *name;
    DrtType type;
} drt_table[NUM_DIRECTIVES] = {
    {"org", DIRECTIVE_ORG},
    {"db", DIRECTIVE_DB},
    {"equ", DIRECTIVE_EQU},
    {"include", DIRECTIVE_INCLUDE},
    {"incbin", DIRECTIVE_INCBIN},
    {"end", DIRECTIVE_END},
};

static Token expect(Lex *lex, TokenType t) {
    Token tk = lex_next(lex);
    if (tk.type != t) {
        fprintf(stderr, "%s expected, got %s at %zu:%zu\n", lex_token2str(t), lex_token2str(tk.type), tk.line, tk.column);
        exit(EXIT_FAILURE);
    }

    return tk;
}

static Stmt parser_directive_stmt(Lex *lex) {
    Token tk = expect(lex, TK_IDENTIFIER);
    str_to_lower(tk.seminfo.id);

    Stmt stmt = {.type=STATEMENT_DIRECTIVE};

    for (size_t k = 0; k < NUM_DIRECTIVES; k++) {
        if (strcmp(drt_table[k].name, tk.seminfo.id) == 0)
            stmt.drt.type = drt_table[k].type;
        else if (k + 1 >= NUM_DIRECTIVES) stmt.drt.type = DIRECTIVE_UNKNOWN;
    }

    switch (stmt.drt.type) {
        case DIRECTIVE_ORG:
            
            break;

        case DIRECTIVE_DB:
            break;

        case DIRECTIVE_EQU:
            break;

        case DIRECTIVE_INCLUDE:
            break;

        case DIRECTIVE_INCBIN:
            break;

        case DIRECTIVE_END:
            break;

        case TK_UNKNOWN:
        default:
            fprintf(stderr, "unknown directive %zu:%zu: %s\n", tk.line, tk.column, tk.seminfo.id);
            exit(EXIT_FAILURE);
    }

    return stmt;
}

static Stmt parser_label_stmt(Lex *lex) {
    Stmt stmt = {.type=STATEMENT_LABEL};

    Token tk = lex_next(lex);
    stmt.label.name = tk.seminfo.id;

    return stmt;
}

static Stmt parser_stmt(Lex *lex) {
    Token tk = lex_next(lex);

    switch (tk.type) {
        case TK_DOT:
            return parser_directive_stmt(lex);

        case TK_IDENTIFIER: {
            
            break;
        }

        case TK_UNKNOWN:
        default:
            fprintf(stderr, "unexpected %s at %zu:%zu\n", lex_token2str(tk.type), tk.line, tk.column);
            exit(EXIT_FAILURE);
    }
}

int main() {
    return 0;
}
