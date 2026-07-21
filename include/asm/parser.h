#pragma once

#include <stdint.h>
#include "asm/lex.h"

typedef enum {
    DIRECTIVE_UNKNOWN = -1,
    DIRECTIVE_ORG,
    DIRECTIVE_DB,
    DIRECTIVE_EQU,
    DIRECTIVE_INCLUDE,
    DIRECTIVE_INCBIN,
    DIRECTIVE_END
} DrtType;

/* number of directives */
#define NUM_DIRECTIVES (DIRECTIVE_END + 1) // do not count "unknown" (org..end)

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

/* directive statement */
typedef struct {
    DrtType type;

    union {
        struct {
            uint8_t *bytes;
            size_t count;
        } data;
        uint16_t value;
        char *path;
    };
} DrtStmt;

#define NUM_INSTR_OPERANDS 3

typedef enum {
    OPERAND_REG,
    OPERAND_IMMEDIATE,
    OPERAND_ADDR,
    OPERAND_REF
} OpType;

/* instruction statement */
typedef struct {
    Mnemonic mnemonic;

    struct {
        OpType type;
    
        union {
            uint8_t reg; /* register V0..VF (0..15) */
            uint16_t value; /* 12-bit integer value (0x000..0xfff) */
            char *ref; /* reference to some label */
        };
    } operands[NUM_INSTR_OPERANDS];

    size_t op_count;
} InstrStmt;

/* label statement */
typedef struct {
    char *name;
} LabelStmt;

typedef enum {
    STATEMENT_DIRECTIVE,
    STATEMENT_LABEL,
    STATEMENT_INSTR
} StmtType;

typedef struct {
    StmtType type;

    union {
        DrtStmt drt; /* directive */
        InstrStmt instr; /* instruction */
        LabelStmt label; /* label */
    };
} Stmt;

