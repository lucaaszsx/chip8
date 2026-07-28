#pragma once

#include "buffer.h"
#include "symbol.h"
#include "ast.h"

typedef struct {
    ByteBuffer buffer;
    SymbolTable *table;
    uint16_t pc;
} CG;

void cg_init(CG *cg, SymbolTable *table);
void cg_free(CG *cg);
void cg_emit_stmt(CG *cg, Stmt stmt);
