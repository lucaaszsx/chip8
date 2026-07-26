#include <stdlib.h>
#include <stdio.h>
#include "semantics.h"
#include "symbol.h"
#include "ast.h"

static uint16_t resolve_value(Semantics *sem, Value value, const char *ctx) {
    if (value.type == VALUE_IMMEDIATE)
        return value.value;

    uint16_t address;
    if (!symbol_lookup(sem->table, value.ref, &address)) {
        fprintf(stderr, "%s: reference to undefined symbol \"%s\"\n", ctx, value.ref);
        exit(EXIT_FAILURE);
    }

    return address;
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
                    sem->pc = resolve_value(sem, stmt.drt.org, "org target");
                    break;

                case DIRECTIVE_EQU: {
                    uint16_t address = resolve_value(sem, stmt.drt.equ.value, "equ value");
                    if (!symbol_define(sem->table, SYMBOL_EQU, stmt.drt.equ.name, sem->pc)) {
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
}

void sem_check(Semantics *sem, Stmt stmt) {}