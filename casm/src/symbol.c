#include <stdint.h>
#include <string.h>
#include "symbol.h"
#include "util.h"

/* initial quantity to be allocated for symbols */
#define SYMBOL_TABLE_ICAP 16

void symbol_tinit(SymbolTable *table) {
    table->capacity = SYMBOL_TABLE_ICAP;
    table->count = 0;
    table->symbols = xmalloc(table->capacity * sizeof(Symbol));
}

void symbol_tfree(SymbolTable *table) {
    free(table->symbols);

    table->symbols = NULL;
    table->capacity = 0;
    table->count = 0;
}

bool symbol_define(SymbolTable *table, SymbolType type, const char *name, uint16_t address) {
    if (symbol_lookup(table, name, NULL))
        return false;

    if (table->count == table->capacity) {
        table->capacity *= 2;
        table->symbols = xrealloc(table->symbols, table->capacity * sizeof(Symbol));
    }

    table->symbols[table->count++] = (Symbol){
        .name=name,
        .type=type,
        .address=address
    };

    return true;
}

bool symbol_lookup(const SymbolTable *table, const char *name, uint16_t *out) {
    for (size_t i = 0; i < table->count; i++) {
        Symbol *sym = &table->symbols[i];
        if (strcmp(sym->name, name) == 0) {
            if (out != NULL) *out = sym->address;
            return true;
        }
    }
    return false;
}

bool symbol_resolve_value(const SymbolTable *table, const Value value, uint16_t *out) {
    if (value.type == VALUE_IMMEDIATE) {
        *out = value.value;
        return true;
    }
    if (!symbol_lookup(table, value.ref, out))
        return false;
    return true;
}
