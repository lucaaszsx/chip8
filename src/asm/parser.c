#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "asm/parser.h"
#include "asm/lex.h"
#include "util.h"

static const DirectiveTable directive_table[DIRECTIVES] = {
    {"org", DIRECTIVE_ORG},
    {"db", DIRECTIVE_DB},
    {"dw", DIRECTIVE_DW},
    {"equ", DIRECTIVE_EQU},
    {"ascii", DIRECTIVE_ASCII},
    {"space", DIRECTIVE_SPACE},
    {"include", DIRECTIVE_INCLUDE},
    {"incbin", DIRECTIVE_INCBIN},
    {"end", DIRECTIVE_END},
};

static Statement *parser_stmt(struct Parser *parser);
static Statement *parser_directive_stmt(struct Parser *parser);
static Statement *parser_label_stmt(struct Parser *parser);
static Statement *parser_create_stmt(StatementType type, Statement **out);
static DirectiveType get_directive_type(char *raw);

void parser_parse() {
    
}

static Statement *parser_stmt(struct Parser *parser) {
    struct Token *tk = lex_next_token(parser->lex);

    switch (tk->type) {
        case TK_DOT:
            return parser_directive_stmt(parser);

        case TK_IDENTIFIER:
            return parser_label_stmt(parser);

        default:
            fprintf(stderr, "unexpected token at %zu:%zu: %s\n", tk->line, tk->column, tk->raw);
            exit(EXIT_FAILURE);
    }
}

static Statement *parser_directive_stmt(struct Parser *parser) {
    Statement *stmt;
    parser_create_stmt(STATEMENT_DIRECTIVE, &stmt);

    // gets directive name
    lex_expect(parser->lex, TK_IDENTIFIER);
    stmt->directive.type = get_directive_type(parser->lex->current->raw);

    // gets directive value
    lex_expect(parser->lex, TK_NUMBER);
    stmt->directive.value = parser->lex->current->value;
}

static Statement *parser_label_stmt(struct Parser *parser) {
    Statement *stmt;
    parser_create_stmt(STATEMENT_LABEL, &stmt);

    lex_expect(parser->lex, TK_IDENTIFIER);
    stmt->label.name = parser->lex->current->raw;

    return stmt;
}

static Statement *parser_create_stmt(StatementType type, Statement **out) {
    Statement *tmp = malloc(sizeof(Statement));
    if (tmp == NULL) {
        fprintf(stderr, "memory allocation for a statement failed\n");
        exit(EXIT_FAILURE);
    }

    tmp->type = type;

    *out = tmp;
}

static DirectiveType get_directive_type(char *raw) {
    char *directive = str_to_lower(raw);

    for (size_t k = 0; k < DIRECTIVES; k++) {
        if (directive_table[k].name == directive)
            return directive_table[k].type;
    }

    return DIRECTIVE_UNKNOWN;
}
