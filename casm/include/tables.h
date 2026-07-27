#pragma once

#include <stdbool.h>
#include "ast.h"

typedef struct {
    char *name;
    DirectiveType type;
} DirectiveEntry;

typedef enum {
    OPKIND_REG = 1 << 0,
    OPKIND_ADDR = 1 << 1,
    OPKIND_BYTE = 1 << 2,
    OPKIND_NIBBLE = 1 << 3
} OpKind;

typedef struct {
    char *name;
    MnemonicType type;
    OpKind ops[NUM_INSTR_OPS];
    size_t expected_ops;
} MnemonicEntry;

extern const DirectiveEntry drt_table[];
extern const MnemonicEntry mnemonic_table[];

DirectiveType get_directive_type(char *s);
bool get_mnemonic(char *s, MnemonicType type, MnemonicEntry *out);
