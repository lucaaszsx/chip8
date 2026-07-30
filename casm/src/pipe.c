#include <stdio.h>
#include "pipe.h"

ByteBuffer *pipe_run(Pipe *pipe, const char *src) {
    arena_init(&pipe->arena);
    lex_init(&pipe->lex, &pipe->arena, src);
    parser_init(&pipe->parser, &pipe->lex);
    symbol_tinit(&pipe->table);
    sem_init(&pipe->sem, &pipe->table);
    cg_init(&pipe->cg, &pipe->table);

    // - step 01: fetch all statements and fill symbol table
    parser_all(&pipe->parser);

    Stmt *stmts = pipe->parser.buffer.data;
    size_t stmt_count = pipe->parser.buffer.size;

    for (size_t i = 0; i < stmt_count; i++)
        sem_collect(&pipe->sem, stmts[i]);

    // -- step 02: check statements
    for (size_t i = 0; i < stmt_count; i++)
        sem_check(&pipe->sem, stmts[i]);

    // --- step 03: generate opcodes
    for (size_t i = 0; i < stmt_count; i++)
        cg_emit_stmt(&pipe->cg, stmts[i]);

    return &pipe->cg.buffer;
}

void pipe_free(Pipe *pipe) {
    cg_free(&pipe->cg);
    symbol_tfree(&pipe->table);
    parser_free(&pipe->parser);
    arena_free(&pipe->arena);
}