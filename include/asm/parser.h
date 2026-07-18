#pragma once

#include <stdint.h>
#include "asm/lex.h"

#define DIRECTIVES 9 // do not count "unknown"

typedef enum {
    STATEMENT_DIRECTIVE,
    STATEMENT_LABEL,
    STATEMENT_INSTR
} StatementType;

typedef enum {
    DIRECTIVE_UNKNOWN = -1,
    DIRECTIVE_ORG,
    DIRECTIVE_DB,
    DIRECTIVE_DW,
    DIRECTIVE_EQU,
    DIRECTIVE_ASCII,
    DIRECTIVE_SPACE,
    DIRECTIVE_INCLUDE,
    DIRECTIVE_INCBIN,
    DIRECTIVE_END
} DirectiveType;

typedef struct {
    const char *name;
    DirectiveType type;
} DirectiveTable;

typedef enum {
    MNEMONIC_CLS,
    MNEMONIC_RTS,
    MNEMONIC_JMP,
    MNEMONIC_JSR,
    MNEMONIC_SKEQ,
    MNEMONIC_SKNE,
    MNEMONIC_MOV,
    MNEMONIC_ADD,
    MNEMONIC_SUB,
    MNEMONIC_OR,
    MNEMONIC_AND,
    MNEMONIC_XOR,
    MNEMONIC_SHR,
    MNEMONIC_SHL,
    MNEMONIC_MVI,
    MNEMONIC_JMI,
    MNEMONIC_RAND,
    MNEMONIC_DRAW,
    MNEMONIC_SKPR,
    MNEMONIC_SKUP,
    MNEMONIC_GDELAY,
    MNEMONIC_SDELAY,
    MNEMONIC_SSOUND,
    MNEMONIC_ADI,
    MNEMONIC_KEY,
    MNEMONIC_FONT,
    MNEMONIC_BCD,
    MNEMONIC_STR,
    MNEMONIC_LDR
} Mnemonic;

typedef enum {
    OPERAND_REG,
    OPERAND_IMMEDIATE,
    OPERAND_ADDR,
    OPERAND_ID
} OperandType;

typedef struct {
    OperandType type;

    union {
        uint8_t reg;
        uint16_t value;
        char *id;
    };
} Operand;

typedef struct {
    DirectiveType type;
    uint16_t value;
} DirectiveStatement;

typedef struct {
    Mnemonic mnemonic;
    Operand operands[2];
    uint8_t op_count;
} InstrStatement;

typedef struct {
    char *name;
} LabelStatement;

typedef struct {
    StatementType type;

    union {
        DirectiveStatement directive;
        InstrStatement instr;
        LabelStatement label;
    };
} Statement;

struct Parser {
    struct Lex *lex;
    Statement stmt;
};
