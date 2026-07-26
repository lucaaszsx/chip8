#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "arena.h"
#include "lex.h"
#include "util.h"

/* initial quantity to be allocated for reading bytes in the db directive */
#define DB_READ_BYTES_ICAP 32

#define is_eol(t) (t.type == TK_NEWLINE || t.type == TK_EOS)

/* Directives table */
static const struct {
    char *name;
    DrtType type;
} drt_table[] = {
    {"org", DIRECTIVE_ORG},
    {"db", DIRECTIVE_DB},
    {"equ", DIRECTIVE_EQU},
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
typedef struct {
    char *name;
    Mnemonic mnemonic;
    size_t expected_ops;
} MnemonicEntry;

static const MnemonicEntry mnemonic_table[] = {
    {"cls", MNEMONIC_CLS, 0},
    {"rts", MNEMONIC_RTS, 0},
    {"jmp", MNEMONIC_JMP, 1},
    {"jsr", MNEMONIC_JSR, 1},
    {"skeq", MNEMONIC_SKEQ, 2},
    {"skne", MNEMONIC_SKNE, 2},
    {"mov", MNEMONIC_MOV, 2},
    {"add", MNEMONIC_ADD, 2},
    {"sub", MNEMONIC_SUB, 2},
    {"or", MNEMONIC_OR, 2},
    {"and", MNEMONIC_AND, 2},
    {"xor", MNEMONIC_XOR, 2},
    {"shr", MNEMONIC_SHR, 1},
    {"shl", MNEMONIC_SHL, 1},
    {"mvi", MNEMONIC_MVI, 1},
    {"jmi", MNEMONIC_JMI, 2},
    {"rand", MNEMONIC_RAND, 2},
    {"draw", MNEMONIC_DRAW, 3},
    {"skpr", MNEMONIC_SKPR, 1},
    {"skup", MNEMONIC_SKUP, 1},
    {"gdelay", MNEMONIC_GDELAY, 1},
    {"sdelay", MNEMONIC_SDELAY, 1},
    {"ssound", MNEMONIC_SSOUND, 1},
    {"adi", MNEMONIC_ADI, 1},
    {"key", MNEMONIC_KEY, 1},
    {"font", MNEMONIC_FONT, 1},
    {"bcd", MNEMONIC_BCD, 1},
    {"str", MNEMONIC_STR, 1},
    {"ldr", MNEMONIC_LDR, 1}
};

/* number of mnemonics */
#define NUM_MNEMONICS (sizeof(mnemonic_table) / sizeof(mnemonic_table[0]))

static int get_mnemonic_idx(char *s) {
    for (size_t k = 0; k < NUM_MNEMONICS; k++) {
        if (istrcasecmp(mnemonic_table[k].name, s) == 0)
            return k;
    }
    return -1;
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

static Value parser_value(Lex *lex) {
    Token tk = lex_next(lex);

    switch (tk.type) {
        case TK_IDENTIFIER:
            return (Value){.type=VALUE_REF, .ref=tk.seminfo.id};

        case TK_NUMBER:
            return (Value){.type=VALUE_IMMEDIATE, .value=tk.seminfo.i};

        default:
            fprintf(stderr, "expected a immediate value or a identifier, got %s at %zu:%zu\n", lex_token2str(tk.type), tk.line, tk.column);
            exit(EXIT_FAILURE);
    }
}

static void parser_read_bytes(Lex *lex, uint8_t **out_bytes, size_t *out_count) {
    size_t capacity = DB_READ_BYTES_ICAP;
    uint8_t *tmp = malloc(capacity * sizeof(uint8_t));
    size_t count = 0;

    for (;;) {
        if (count == capacity) {
            capacity *= 2;
            tmp = realloc(tmp, capacity * sizeof(uint8_t));
        }

        Token tk = parser_expect(lex, TK_NUMBER);
        uint16_t value = tk.seminfo.i;

        if (value > UINT8_MAX) {
            fprintf(stderr, "invalid byte at %zu:%zu\n", tk.line, tk.column);
            exit(EXIT_FAILURE);
        }

        tmp[count++] = (uint8_t)value;

        Token next = lex_lookahead(lex);
        if (is_eol(next)) break;

        parser_expect(lex, TK_COMMA); // consume TK_COMMA (,) before reading the next byte
    }

    *out_bytes = arena_memcpy(lex->arena, tmp, count);
    free(tmp);

    *out_count = count;
}

static Stmt parser_directive_stmt(Lex *lex) {
    Stmt stmt = {.type=STATEMENT_DIRECTIVE, .line=lex->line};
    Token tk = parser_expect(lex, TK_IDENTIFIER);

    switch ((stmt.drt.type = get_drt_type(tk.seminfo.id))) {
        case DIRECTIVE_ORG:
            stmt.drt.org = parser_value(lex);
            break;

        case DIRECTIVE_DB:
            parser_read_bytes(lex, &stmt.drt.db.bytes, &stmt.drt.db.count);
            break;

        case DIRECTIVE_EQU:
            stmt.drt.equ.name = parser_expect(lex, TK_IDENTIFIER).seminfo.id;
            stmt.drt.equ.value = parser_value(lex);
            break;

        case DIRECTIVE_END:
            /* does nothing */
            break;

        default:
            fprintf(stderr, "unknown directive %zu:%zu: %s\n", tk.line, tk.column, tk.seminfo.id);
            exit(EXIT_FAILURE);
    }

    return stmt;
}

static Stmt parser_instr_stmt(Lex *lex, size_t idx) {
    MnemonicEntry entry = mnemonic_table[idx];

    Stmt stmt = {.type=STATEMENT_INSTR, .line=lex->line};
    stmt.instr.mnemonic = entry.mnemonic;
    stmt.instr.op_count = 0;

    Token next = lex_lookahead(lex);
    if (is_eol(next)) goto check_count;

    for (;;) {
        Token tk = lex_next(lex);

        if (stmt.instr.op_count == NUM_INSTR_OPS) {
            fprintf(stderr, "too many operands at %zu:%zu\n", tk.line, tk.column);
            exit(EXIT_FAILURE);
        }

        OpType type;
        if (tk.type == TK_REGISTER)
            type = OPERAND_REG;
        else if (tk.type == TK_IDENTIFIER || tk.type == TK_NUMBER)
            type = OPERAND_VALUE;
        else {
            fprintf(stderr, "unexpected %s in operand list at %zu:%zu\n", lex_token2str(tk.type), tk.line, tk.column);
            exit(EXIT_FAILURE);
        }

        stmt.instr.operands[stmt.instr.op_count].type = type;
        if (type == OPERAND_REG)
            stmt.instr.operands[stmt.instr.op_count].reg = tk.seminfo.r;
        else if (tk.type == TK_IDENTIFIER)
            stmt.instr.operands[stmt.instr.op_count].value = (Value){.type=VALUE_REF, .ref=tk.seminfo.id};
        else
            stmt.instr.operands[stmt.instr.op_count].value = (Value){.type=VALUE_IMMEDIATE, .value=tk.seminfo.i};

        stmt.instr.op_count++;

        next = lex_lookahead(lex);
        if (is_eol(next)) break;

        parser_expect(lex, TK_COMMA);
    }

    check_count:
    if (stmt.instr.op_count != entry.expected_ops) {
        fprintf(stderr, "%s expects %zu operands, got %zu at %zu:%zu\n", entry.name, entry.expected_ops, stmt.instr.op_count, next.line, next.column);
        exit(EXIT_FAILURE);
    }
    
    return stmt;
}

bool parser_stmt(Lex *lex, Stmt *out) {
    Token tk;
    do {
        tk = lex_next(lex);
    } while (tk.type == TK_NEWLINE);

    if (tk.type == TK_EOS) return false;

    switch (tk.type) {
        case TK_DOT:
            *out = parser_directive_stmt(lex);
            break;

        case TK_IDENTIFIER: {
            int mnemonic_idx;

            if (lex_lookahead(lex).type == TK_COLON) {
                lex_next(lex); // consume TK_COLON (:)
                *out = (Stmt){
                    .type=STATEMENT_LABEL,
                    .label=(LabelStmt){.name=tk.seminfo.id},
                    .line=lex->line
                };
                break;
            } else if ((mnemonic_idx = get_mnemonic_idx(tk.seminfo.id)) > -1) {
                *out = parser_instr_stmt(lex, mnemonic_idx);
                break;
            }

            fprintf(stderr, "unexpected identifier \"%s\" at %zu:%zu\n", tk.seminfo.id, tk.line, tk.column);
            exit(EXIT_FAILURE);
        }

        case TK_NEWLINE:
            /* does nothing */
            break;

        default:
            fprintf(stderr, "unexpected %s at %zu:%zu\n", lex_token2str(tk.type), tk.line, tk.column);
            exit(EXIT_FAILURE);
    }

    Token next = lex_next(lex);
    if (!is_eol(next)) {
        fprintf(stderr, "unexpected %s at the end of the statement at %zu:%zu\n", lex_token2str(next.type), next.line, next.column);
        exit(EXIT_FAILURE);
    }

    return true;
}
