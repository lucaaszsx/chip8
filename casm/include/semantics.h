#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "symbol.h"
#include "ast.h"

typedef struct {
    uint16_t pc;
    SymbolTable *table;
} Semantics;

void sem_collect(Semantics *sem, Stmt stmt);
void sem_check(Semantics *sem, Stmt stmt);
