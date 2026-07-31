#pragma once

#include "semantics.h"
#include "symbol.h"
#include "parser.h"
#include "buffer.h"
#include "arena.h"
#include "lex.h"

typedef struct {
    ArenaAllocator arena;
    Lex lex;
    Parser parser;
    SymbolTable table;
    Semantics sem;
} Pipe;

void pipe_run(Pipe *pipe, const char *src, ByteBuffer *out);
void pipe_free(Pipe *pipe);
