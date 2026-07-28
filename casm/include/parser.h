#pragma once

#include "buffer.h"
#include "lex.h"

typedef struct {
    StmtBuffer buffer;
    Lex *lex;
} Parser;

void parser_init(Parser *parser, Lex *lex);
void parser_free(Parser *parser);
void parser_all(Parser *parser);
