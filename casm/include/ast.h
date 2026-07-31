#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum {
    MNEMONIC_UNKNOWN = -1,
    MNEMONIC_CLS,
    MNEMONIC_RTS,
    MNEMONIC_JMP,
    MNEMONIC_JSR,
    MNEMONIC_SKEQ,
    MNEMONIC_SKNE,
    MNEMONIC_MOV,
    MNEMONIC_ADD,
    MNEMONIC_SUB,
    MNEMONIC_RSB,
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
} MnemonicType;

typedef enum {
    VALUE_IMMEDIATE,
    VALUE_REF
} ValueType;

/* type for immediate values and references that needs to be resolved */
typedef struct {
    ValueType type;

    union {
        uint16_t value; /* 12-bit immediate integer value (0x000..0xfff) */
        char *ref; /* reference to some label/constant */
    };
} Value;

typedef enum {
    DIRECTIVE_UNKNOWN = -1,
    DIRECTIVE_ORG,
    DIRECTIVE_DB,
    DIRECTIVE_EQU,
    DIRECTIVE_END
} DirectiveType;

/* directive statement */
typedef struct {
    DirectiveType type;

    union {
        Value org; /* org */

        struct {
            uint8_t *bytes;
            size_t count;
        } db; /* db */

        struct {
            char *name;
            Value value;
        } equ; /* equ */
    };
} DrtStmt;

/* max operands supported by instructions */
#define NUM_INSTR_OPS 3

typedef enum {
    OPERAND_REG,
    OPERAND_VALUE
} OpType;

typedef struct {
    OpType type;

    union {
        uint8_t reg; /* register V0..VF (0..15) */
        Value value;
    };
} Operand;

/* instruction statement */
typedef struct {
    MnemonicType type;
    Operand operands[NUM_INSTR_OPS];
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

    size_t line; /* statement line */
} Stmt;

/* public method to get a statement size and correctly increments the location counter (or PC/program counter) */
size_t ast_stmt_size(const Stmt stmt);
