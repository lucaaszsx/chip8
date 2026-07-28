#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "semantics.h"
#include "tables.h"
#include "symbol.h"
#include "ast.h"

void sem_init(Semantics *sem, SymbolTable *table) {
    sem->table = table;
    sem->pc = 0;
}

static uint16_t resolve_value(const Semantics *sem, const Value value) {
    uint16_t result;
    if (!symbol_resolve_value(sem->table, value, &result)) {
        fprintf(stderr, "reference to undefined symbol \"%s\"\n", value.ref);
        exit(EXIT_FAILURE);
    }
    return result;
}

void sem_collect(Semantics *sem, Stmt stmt) {
    switch (stmt.type) {
        case STATEMENT_LABEL: {
            if (!symbol_define(sem->table, SYMBOL_LABEL, stmt.label.name, sem->pc)) {
                fprintf(stderr, "duplicate label \"%s\" (line %zu)\n", stmt.label.name, stmt.line);
                exit(EXIT_FAILURE);
            }
            break;
        }

        case STATEMENT_DIRECTIVE: {
            switch (stmt.drt.type) {
                case DIRECTIVE_ORG:
                    sem->pc = resolve_value(sem, stmt.drt.org);
                    break;

                case DIRECTIVE_EQU: {
                    uint16_t value = resolve_value(sem, stmt.drt.equ.value);
                    if (!symbol_define(sem->table, SYMBOL_EQU, stmt.drt.equ.name, value)) {
                        fprintf(stderr, "duplicate constant \"%s\" (line %zu)\n", stmt.drt.equ.name, stmt.line);
                        exit(EXIT_FAILURE);
                    }
                    break;
                }

                case DIRECTIVE_UNKNOWN:
                case DIRECTIVE_DB:
                case DIRECTIVE_END:
                    /* theres nothing to be resolved */
                    break;
            }
            return;
        }

        case STATEMENT_INSTR:
            /* nothing to collect for symbol table in instructions */
            break;
    }

    sem->pc += ast_stmt_size(stmt);
}

/* enum with every immediate value combination */
enum {
    OPKIND_IMMEDIATE_MASK =
        OPKIND_NIBBLE |
        OPKIND_BYTE |
        OPKIND_ADDR
};

static bool op_range_check(OpKind kind, uint16_t value) {
    assert((kind & OPKIND_IMMEDIATE_MASK) != 0); // at least one immediate kind flag must be present
    assert((kind & ~(OPKIND_IMMEDIATE_MASK | OPKIND_REG)) == 0); // dont allow unknown flags

    return ((kind & OPKIND_NIBBLE) != 0 && value <= 0xf) ||
        ((kind & OPKIND_BYTE) != 0 && value <= 0xff) ||
        ((kind & OPKIND_ADDR) != 0 && value <= 0xfff);
}

void sem_check(Semantics *sem, Stmt stmt) {
    if (stmt.type != STATEMENT_INSTR) return;

    MnemonicEntry mnemonic;
    assert(get_mnemonic(NULL, stmt.instr.type, &mnemonic));
    assert(stmt.instr.op_count == mnemonic.expected_ops); // the parser must have picked up the correct number of operands

    for (size_t o = 0; o < stmt.instr.op_count; o++) {
        const Operand op = stmt.instr.operands[o];
        OpKind expected = mnemonic.ops[o];

        switch (op.type) {
            case OPERAND_VALUE: {
                if ((expected & OPKIND_IMMEDIATE_MASK) == 0) {
                    fprintf(stderr, "unexpected value in operand %zu in instruction %s at line %zu\n", o + 1, mnemonic.name, stmt.line);
                    exit(EXIT_FAILURE);
                }

                uint16_t value = resolve_value(sem, op.value);
                if (!op_range_check(expected, value)) {
                    fprintf(stderr, "operand %zu of %s in line %zu is out-of-range\n", o + 1, mnemonic.name, stmt.line);
                    exit(EXIT_FAILURE);
                }
                break; // just so the compiler doesn't complain
            }

            case OPERAND_REG: {
                if ((expected & OPKIND_REG) == 0) {
                    fprintf(stderr, "unexpected register in operand %zu in instruction %s at line %zu\n", o + 1, mnemonic.name, stmt.line);
                    exit(EXIT_FAILURE);
                }
                break; // ...
            }
        }
    }
}
