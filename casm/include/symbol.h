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
    size_t capacity;
} SymbolTable;

void symbol_tinit(SymbolTable *table);
void symbol_tfree(SymbolTable *table);
bool symbol_define(SymbolTable *table, SymbolType type, const char *name, uint16_t address);
bool symbol_lookup(SymbolTable *table, const char *name, uint16_t *out);
