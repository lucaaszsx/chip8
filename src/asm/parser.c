#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "asm/parser.h"
#include "asm/arena.h"
#include "asm/lex.h"
#include "util.h"

/* Directives table */
static const struct {
    char *name;
    DrtType type;
} drt_table[] = {
    {"org", DIRECTIVE_ORG},
    {"db", DIRECTIVE_DB},
    {"equ", DIRECTIVE_EQU},
    {"include", DIRECTIVE_INCLUDE},
    {"incbin", DIRECTIVE_INCBIN},
    {"end", DIRECTIVE_END},
};

/* number of directives */
#define NUM_DIRECTIVES (sizeof(drt_table) / sizeof(drt_table[0]))

static DrtType get_drt_type(char *s) {
    for (size_t k = 0; k < NUM_DIRECTIVES; k++) {
        if (istrcasecmp(drt_table[k].name, s) == 0)
            return drt_table[k].type;
    }
    return DIRECTIVE_UNKNOWN;
}

/* Mnemonics table */
static const struct {
    char *name;
    Mnemonic mnemonic;
} mnemonic_table[] = {
    {"cls", MNEMONIC_CLS},
    {"rts", MNEMONIC_RTS},
    {"jmp", MNEMONIC_JMP},
    {"jsr", MNEMONIC_JSR},
    {"skeq", MNEMONIC_SKEQ},
    {"skne", MNEMONIC_SKNE},
    {"mov", MNEMONIC_MOV},
    {"add", MNEMONIC_ADD},
    {"sub", MNEMONIC_SUB},
    {"or", MNEMONIC_OR},
    {"and", MNEMONIC_AND},
    {"xor", MNEMONIC_XOR},
    {"shr", MNEMONIC_SHR},
    {"shl", MNEMONIC_SHL},
    {"mvi", MNEMONIC_MVI},
    {"jmi", MNEMONIC_JMI},
    {"rand", MNEMONIC_RAND},
    {"draw", MNEMONIC_DRAW},
    {"skpr", MNEMONIC_SKPR},
    {"skup", MNEMONIC_SKUP},
    {"gdelay", MNEMONIC_GDELAY},
    {"sdelay", MNEMONIC_SDELAY},
    {"ssound", MNEMONIC_SSOUND},
    {"adi", MNEMONIC_ADI},
    {"key", MNEMONIC_KEY},
    {"font", MNEMONIC_FONT},
    {"bcd", MNEMONIC_BCD},
    {"str", MNEMONIC_STR},
    {"ldr", MNEMONIC_LDR}
};

/* number of mnemonics */
#define NUM_MNEMONICS (sizeof(mnemonic_table) / sizeof(mnemonic_table[0]))

static Mnemonic get_mnemonic(char *s) {
    for (size_t k = 0; k < NUM_MNEMONICS; k++) {
        if (istrcasecmp(mnemonic_table[k].name, s) == 0)
            return mnemonic_table[k].mnemonic;
    }
    return MNEMONIC_UNKNOWN;
}

//
// PARSER
//

static Token parser_expect(Lex *lex, TokenType t) {
    Token tk = lex_next(lex);
    if (tk.type != t) {
        fprintf(stderr, "%s expected, got %s at %zu:%zu\n", lex_token2str(t), lex_token2str(tk.type), tk.line, tk.column);
        exit(EXIT_FAILURE);
    }

    return tk;
}

static Expr parser_expr(Lex *lex) {
    Token tk = lex_next(lex);

    switch (tk.type) {
        case TK_IDENTIFIER:
            return (Expr){.type=EXPR_REF, .ref=tk.seminfo.id};

        case TK_NUMBER:
            return (Expr){.type=EXPR_IMMEDIATE, .value=tk.seminfo.i};

        default:
            fprintf(stderr, "expected a immediate value or a identifier, got %s\n", lex_token2str(tk.type));
            exit(EXIT_FAILURE);
    }
}

static Stmt parser_directive_stmt(Lex *lex) {
    lex_next(lex); // consume "." (TK_DOT)

    Stmt stmt = {.type=STATEMENT_DIRECTIVE};
    Token tk = parser_expect(lex, TK_IDENTIFIER);

    switch ((stmt.drt.type = get_drt_type(tk.seminfo.id))) {
        case DIRECTIVE_ORG:
            stmt.drt.expr = parser_expr(lex);
            return stmt;

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

        default:
            fprintf(stderr, "unknown directive %zu:%zu: %s\n", tk.line, tk.column, tk.seminfo.id);
            exit(EXIT_FAILURE);
    }

    return stmt;
}

static Stmt parser_instr_stmt(Lex *lex, Mnemonic m) {
    
}

static Stmt parser_label_stmt(Lex *lex) {
    Stmt stmt = {.type=STATEMENT_LABEL};

    Token tk = lex_next(lex);
    stmt.label.name = tk.seminfo.id;

    return stmt;
}

static Stmt parser_stmt(Lex *lex) {
    Token tk = lex_lookahead(lex);

    switch (tk.type) {
        case TK_DOT:
            return parser_directive_stmt(lex);

        case TK_IDENTIFIER: {
            if ()
            if (next.type == TK_COLON)
                return parser_label_stmt(lex);
        }

        case TK_UNKNOWN:
        default:
            fprintf(stderr, "unexpected %s at %zu:%zu\n", lex_token2str(tk.type), tk.line, tk.column);
            exit(EXIT_FAILURE);
    }
}

int main() {
    static const char *source =
        ".org v0";
    ArenaAllocator arena;
    arena_init(&arena);
    Lex lex;
    lex_init(&lex, &arena, source);

    Stmt stmt = parser_stmt(&lex);
    if (stmt.type == STATEMENT_DIRECTIVE) {
        printf("directive: %d\n", stmt.drt.type);

        switch (stmt.drt.type) {
            case DIRECTIVE_ORG:
                switch (stmt.drt.expr.type) {
                    case EXPR_IMMEDIATE:
                        printf("  .ORG 0x%04X\n", stmt.drt.expr.value);
                        break;
                    case EXPR_REF:
                        printf("  .ORG %s\n", stmt.drt.expr.ref);
                        break;
                }
                break;
        }

        puts("\n");
    }
    return 0;
}
