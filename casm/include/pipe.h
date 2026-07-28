#pragma once

#include "semantics.h"
#include "codegen.h"
#include "symbol.h"
#include "parser.h"
#include "arena.h"
#include "lex.h"

typedef struct {
    ArenaAllocator arena;
    Lex lex;
    Parser parser;
    SymbolTable table;
    Semantics sem;
    CG cg;
} Pipe;

void pipe_run(Pipe *pipe, const char *src);
