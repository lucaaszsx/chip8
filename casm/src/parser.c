#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"
#include "buffer.h"
#include "tables.h"
#include "parser.h"
#include "arena.h"
#include "lex.h"

/* initial quantity to be allocated for storing statements */
#define STMT_BUFFER_ICAP 64

/* initial quantity to be allocated for reading bytes in the db directive */
#define DB_READ_BYTES_ICAP 32

#define is_eol(t) (t.type == TK_NEWLINE || t.type == TK_EOS)

//
// PARSER
//

void parser_init(Parser *parser, Lex *lex) {
    stmt_buf_init(&parser->buffer, STMT_BUFFER_ICAP);
    parser->lex = lex;
    parser->ended = false;
}

void parser_free(Parser *parser) {
    stmt_buf_free(&parser->buffer);
}

static Token parser_expect(Parser *parser, TokenType t) {
    Token tk = lex_next(parser->lex);
    if (tk.type != t) {
        fprintf(stderr, "%s expected, got %s at %zu:%zu\n", lex_token2str(t), lex_token2str(tk.type), tk.line, tk.column);
        exit(EXIT_FAILURE);
    }
    return tk;
}

static Value parser_value(Parser *parser) {
    Token tk = lex_next(parser->lex);

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

static void parser_read_bytes(Parser *parser, uint8_t **out_bytes, size_t *out_count) {
    ByteBuffer tmp;
    buf_init(&tmp, DB_READ_BYTES_ICAP);

    for (size_t i = 1; ; i++) {
        Token tk = parser_expect(parser, TK_NUMBER);
        uint16_t value = tk.seminfo.i;

        if (value > UINT8_MAX) {
            fprintf(stderr, "invalid byte in position %zu at %zu:%zu\n", i, tk.line, tk.column);
            exit(EXIT_FAILURE);
        }

        buf_write_u8(&tmp, value);

        Token next = lex_lookahead(parser->lex);
        if (is_eol(next)) break;

        parser_expect(parser, TK_COMMA); // consume TK_COMMA (,) before reading the next byte
    }

    *out_bytes = arena_memcpy(parser->lex->arena, tmp.data, tmp.size);
    *out_count = tmp.size;

    buf_free(&tmp);
}

static Stmt parser_directive_stmt(Parser *parser) {
    Stmt stmt = {.type=STATEMENT_DIRECTIVE, .line=parser->lex->line};
    Token tk = parser_expect(parser, TK_IDENTIFIER);

    switch ((stmt.drt.type = get_directive_type(tk.seminfo.id))) {
        case DIRECTIVE_ORG:
            stmt.drt.org = parser_value(parser);
            break;

        case DIRECTIVE_DB:
            parser_read_bytes(parser, &stmt.drt.db.bytes, &stmt.drt.db.count);
            break;

        case DIRECTIVE_EQU:
            stmt.drt.equ.name = parser_expect(parser, TK_IDENTIFIER).seminfo.id;
            stmt.drt.equ.value = parser_value(parser);
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

static Stmt parser_instr_stmt(Parser *parser, MnemonicEntry mnemonic) {
    Stmt stmt = {.type=STATEMENT_INSTR, .line=parser->lex->line};
    stmt.instr.type = mnemonic.type;
    stmt.instr.op_count = 0;

    Token next = lex_lookahead(parser->lex);
    if (is_eol(next)) goto check_count;

    for (;;) {
        Token tk = lex_next(parser->lex);

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

        next = lex_lookahead(parser->lex);
        if (is_eol(next)) break;

        parser_expect(parser, TK_COMMA);
    }

    check_count:
    if (stmt.instr.op_count != mnemonic.expected_ops) {
        fprintf(stderr, "%s expects %zu operands, got %zu at %zu:%zu\n", mnemonic.name, mnemonic.expected_ops, stmt.instr.op_count, next.line, next.column);
        exit(EXIT_FAILURE);
    }
    
    return stmt;
}

static bool parser_stmt(Parser *parser, Stmt *out) {
    Token tk;
    do {
        tk = lex_next(parser->lex);
    } while (tk.type == TK_NEWLINE);

    if (tk.type == TK_EOS) return false;

    switch (tk.type) {
        case TK_DOT:
            *out = parser_directive_stmt(parser);
            break;

        case TK_IDENTIFIER: {
            MnemonicEntry mnemonic;

            if (lex_lookahead(parser->lex).type == TK_COLON) {
                lex_next(parser->lex); // consume TK_COLON (:)
                *out = (Stmt){
                    .type=STATEMENT_LABEL,
                    .label=(LabelStmt){.name=tk.seminfo.id},
                    .line=parser->lex->line
                };
                break;
            } else if (get_mnemonic(tk.seminfo.id, -1, &mnemonic)) {
                *out = parser_instr_stmt(parser, mnemonic);
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

    Token next = lex_next(parser->lex);
    if (!is_eol(next)) {
        fprintf(stderr, "unexpected %s at the end of the statement at %zu:%zu\n", lex_token2str(next.type), next.line, next.column);
        exit(EXIT_FAILURE);
    }

    return true;
}

void parser_all(Parser *parser) {
    Stmt stmt;
    while (parser_stmt(parser, &stmt) && !parser->ended) {
        // stops reading statements when reaches ".end" directive
        if (stmt.type == STATEMENT_DIRECTIVE && stmt.drt.type == DIRECTIVE_END) {
            parser->ended = true;
            return;
        }
        stmt_buf_push(&parser->buffer, stmt);
    }
}