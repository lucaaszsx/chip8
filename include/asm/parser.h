#pragma once

#include <stdint.h>
#include "asm/lex.h"

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
    EXPR_IMMEDIATE,
    EXPR_REF
} ExprType;

/* expression for immediates or values that needs to be resolved */
typedef struct {
    ExprType type;

    union {
        uint16_t value; /* 12-bit immediate integer value (0x000..0xfff) */
        char *ref; /* reference to some label/constant */
    };
} Expr;

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

/* directive statement */
typedef struct {
    DrtType type;

    union {
        Expr expr; /* org */

        struct {
            Expr *bytes;
            size_t count;
        } data; /* db */

        struct {
            char *name;
            Expr value;
        } equ; /* equ */

        char *path; /* include, incbin */
    };
} DrtStmt;

/* max operands supported by instructions */
#define NUM_INSTR_OPS 3

typedef enum {
    OPERAND_REG,
    OPERAND_EXPR
} OpType;

/* instruction statement */
typedef struct {
    Mnemonic mnemonic;

    struct {
        OpType type;
    
        union {
            uint8_t reg; /* register V0..VF (0..15) */
            Expr expr;
        };
    } operands[NUM_INSTR_OPS];

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

