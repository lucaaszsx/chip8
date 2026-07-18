#include "asm/parser.h"
#include "asm/lex.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Statement *parser_stmt(struct Parser *parser);
static Statement *parser_directive_stmt(struct Parser *parser);
static Statement *parser_label_stmt(struct Parser *parser);
static Statement *parser_create_stmt(struct Parser *parser, StatementType type, Statement **out);
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
            fprintf(stderr, "unexpected token at %zu:%zu: %s", tk->line, tk->column, tk->raw);
            exit(EXIT_FAILURE);
    }
}

static Statement *parser_directive_stmt(struct Parser *parser) {
    Statement *stmt;
    parser_create_stmt(parser, STATEMENT_DIRECTIVE, &stmt);

    // gets directive name
    lex_expect(parser->lex, TK_IDENTIFIER);
    stmt->directive.type = get_directive_type(parser->lex->current->raw);

    // gets directive value
    lex_expect(parser->lex, TK_NUMBER);
    stmt->directive.value = parser->lex->current->value;
}

static Statement *parser_label_stmt(struct Parser *parser) {
    Statement *stmt;
    parser_create_stmt(parser, STATEMENT_LABEL, &stmt);

    lex_expect(parser->lex, TK_IDENTIFIER);
    stmt->label.name = parser->lex->current->raw;

    return stmt;
}

static Statement *parser_create_stmt(struct Parser *parser, StatementType type, Statement **out) {
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

    if (strcmp(directive, "org") == 0) return DIRECTIVE_ORG;
    else if (strcmp(directive, "db") == 0) return DIRECTIVE_DB;
    else if (strcmp(directive, "dw") == 0) return DIRECTIVE_DW;
    else if (strcmp(directive, "equ") == 0) return DIRECTIVE_EQU;
    else if (strcmp(directive, "ascii") == 0) return DIRECTIVE_ASCII;
    else if (strcmp(directive, "space") == 0) return DIRECTIVE_SPACE;
    else if (strcmp(directive, "include") == 0) return DIRECTIVE_INCLUDE;
    else if (strcmp(directive, "incbin") == 0) return DIRECTIVE_INCBIN;
    else if (strcmp(directive, "end") == 0) return DIRECTIVE_END;
    else return NULL;
}
