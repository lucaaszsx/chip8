#pragma once

#include "buffer.h"
#include "symbol.h"
#include "ast.h"

void cg_emit_stmt(SymbolTable *table, Stmt stmt, ByteBuffer *out);
