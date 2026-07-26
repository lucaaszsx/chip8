#pragma once

#include "lex.h"
#include "ast.h"

bool parser_stmt(Lex *lex, Stmt *out);
