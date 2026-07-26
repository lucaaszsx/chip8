#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    SYMBOL_LABEL,
    SYMBOL_EQU
} SymbolType;

typedef struct {
    const char *name;
    SymbolType type;
    uint16_t address;
} Symbol;

typedef struct {
    Symbol *symbols;
    size_t count;
    size_t cap;
} SymbolTable;

void symbol_tinit(SymbolTable *);
void symbol_tfree(SymbolTable *);
bool symbol_define(SymbolTable *, SymbolType type, const char *name, uint16_t address);
bool *symbol_lookup(SymbolTable *, const char *name, uint16_t *out);
