#pragma once

#include <stdbool.h>
#include "buffer.h"
#include "lex.h"

typedef struct {
    StmtBuffer buffer;
    Lex *lex;
    bool ended;
} Parser;

void parser_init(Parser *parser, Lex *lex);
void parser_free(Parser *parser);
void parser_all(Parser *parser);
