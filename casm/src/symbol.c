#include <string.h>
#include "symbol.h"

/* initial quantity to be allocated for symbols */
#define SYMBOL_TABLE_ICAP 16

void symbol_tinit(SymbolTable *table) {
    table->cap = SYMBOL_TABLE_ICAP;
    table->count = 0;
    table->symbols = malloc(table->cap * sizeof(Symbol));
}

void symbol_tfree(SymbolTable *table) {
    free(table->symbols);

    table->symbols = NULL;
    table->cap = 0;
    table->count = 0;
}

bool symbol_define(SymbolTable *table, SymbolType type, const char *name, uint16_t address) {
    if (symbol_lookup(table, name) != NULL)
        return false;

    if (table->count == table->cap) {
        table->cap *= 2;

        Symbol *new_symbols = realloc(table->symbols, table->cap * sizeof(Symbol));
        if (new_symbols == NULL) return false;

        table->symbols = new_symbols;
    }

    table->symbols[table->count++] = (Symbol){
        .name=name,
        .type=type,
        .address=address
    };

    return true;
}

Symbol *symbol_lookup(SymbolTable *table, const char *name) {
    for (size_t i = 0; i < table->count; i++) {
        Symbol *sym = &table->symbols[i];
        if (strcmp(sym->name, name) == 0)
            return sym;
    }
    return NULL;
}