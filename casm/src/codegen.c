#include <assert.h>
#include "codegen.h"

/* opcodes */
#define OPCODE_CLS 0x00e0 // 0x00E0
#define OPCODE_RTS 0x00ee // 0x00EE
#define OPCODE_JMP 0x1000 // 0x1NNN
#define OPCODE_JSR 0x2000 // 0x2NNN
#define OPCODE_SKEQ_I 0x3000 // 0x3XNN
#define OPCODE_SKEQ_R 0x5000 // 0x5XY0
#define OPCODE_SKNE_I 0x4000 // 0x4XNN
#define OPCODE_SKNE_R 0x9000 // 0x9XY0
#define OPCODE_MOV_I 0x6000 // 0x6XNN
#define OPCODE_MOV_R 0x8000 // 0x8XY0
#define OPCODE_ADD_I 0x7000 // 0x7XNN
#define OPCODE_ADD_R 0x8004 // 0x8XY4
#define OPCODE_SUB 0x8005 // 0x8XY5
#define OPCODE_RSB 0x8007 // 0x8XY7
#define OPCODE_OR 0x8001 // 0x8XY1
#define OPCODE_AND 0x8002 // 0x8XY2
#define OPCODE_XOR 0x8003 // 0x8XY3
#define OPCODE_SHR 0x8006 // 0x8XY6
#define OPCODE_SHL 0x800e // 0x8XYE
#define OPCODE_MVI 0xa000 // 0xANNN
#define OPCODE_JMI 0xb000 // 0xBXNN
#define OPCODE_RAND 0xc000 // 0xCXNN
#define OPCODE_DRAW 0xd000 // 0xDXYN
#define OPCODE_SKPR 0xe09e // 0xEX9E
#define OPCODE_SKUP 0xe0a1 // 0xEXA1
#define OPCODE_GDELAY 0xf007 // 0xFX07
#define OPCODE_SDELAY 0xf015 // 0xFX15
#define OPCODE_SSOUND 0xf018 // 0xFX18
#define OPCODE_ADI 0xf01e // 0xFX1E
#define OPCODE_KEY 0xf00a // 0xFX0A
#define OPCODE_FONT 0xf029 // 0xFX29
#define OPCODE_BCD 0xf033 // 0xFX33
#define OPCODE_STR 0xf055 // 0xFX55
#define OPCODE_LDR 0xf065 // 0xFX65

static uint16_t append_x(uint16_t word, uint8_t x) {
    assert(x <= 0xf); // nibble check
    return (word & 0xf0ff) | (x << 8);
}

static uint16_t append_xy(uint16_t word, uint8_t x, uint8_t y) {
    assert((x <= 0xf) && (y <= 0xf));
    return (word & 0xf00f) | (x << 8) | (y << 4);
}

static uint16_t append_xyn(uint16_t word, uint8_t x, uint8_t y, uint8_t n) {
    assert((x <= 0xf) && (y <= 0xf) && (n <= 0xf));
    return (word & 0xf000) | (x << 8) | (y << 4) | n;
}

static uint16_t append_xnn(uint16_t word, uint8_t x, uint8_t nn) {
    assert(x <= 0xf);
    return (word & 0xf000) | (x << 8) | nn;
}

static uint16_t append_nnn(uint16_t word, uint16_t nnn) {
    assert(nnn <= 0xfff); // 12-bit number check
    return (word & 0xf000) | nnn;
}

/* resolved operand value (register/immediate/reference) */
typedef struct {
    OpType type;
    uint16_t value;
} ROP;

static uint16_t get_word_dual_op(MnemonicType type, ROP v[]) {
    bool has_gpr = v[1].type == OPERAND_REG;
    uint16_t opcode;

    switch (type) {
        case MNEMONIC_SKEQ:
            opcode = has_gpr ? OPCODE_SKEQ_R : OPCODE_SKEQ_I;
            break;

        case MNEMONIC_SKNE:
            opcode = has_gpr ? OPCODE_SKNE_R : OPCODE_SKNE_I;
            break;

        case MNEMONIC_MOV:
            opcode = has_gpr ? OPCODE_MOV_R : OPCODE_MOV_I;
            break;

        case MNEMONIC_ADD:
            opcode = has_gpr ? OPCODE_ADD_R : OPCODE_ADD_I;
            break;

        default:
            assert(0 && "mnemonic without double operands");
    }

    if (has_gpr) return append_xy(opcode, v[0].value, v[1].value);
    else return append_xnn(opcode, v[0].value, v[1].value);
}

static uint16_t get_word(MnemonicType type, ROP v[]) {
    switch (type) {
        case MNEMONIC_CLS:
            return OPCODE_CLS;

        case MNEMONIC_RTS:
            return OPCODE_RTS;

        case MNEMONIC_JMP:
            return append_nnn(OPCODE_JMP, v[0].value);

        case MNEMONIC_JSR:
            return append_nnn(OPCODE_JSR, v[0].value);

        case MNEMONIC_SUB:
            return append_xy(OPCODE_SUB, v[0].value, v[1].value);

        case MNEMONIC_RSB:
            return append_xy(OPCODE_RSB, v[0].value, v[1].value);

        case MNEMONIC_OR:
            return append_xy(OPCODE_OR, v[0].value, v[1].value);

        case MNEMONIC_AND:
            return append_xy(OPCODE_AND, v[0].value, v[1].value);

        case MNEMONIC_XOR:
            return append_xy(OPCODE_XOR, v[0].value, v[1].value);

        case MNEMONIC_SHR:
            return append_x(OPCODE_SHR, v[0].value);

        case MNEMONIC_SHL:
            return append_x(OPCODE_SHL, v[0].value);

        case MNEMONIC_MVI:
            return append_nnn(OPCODE_MVI, v[0].value);

        case MNEMONIC_JMI:
            return append_xnn(OPCODE_JMI, v[0].value, v[1].value);

        case MNEMONIC_RAND:
            return append_xnn(OPCODE_RAND, v[0].value, v[1].value);

        case MNEMONIC_DRAW:
            return append_xyn(OPCODE_DRAW, v[0].value, v[1].value, v[2].value);

        case MNEMONIC_SKPR:
            return append_x(OPCODE_SKPR, v[0].value);

        case MNEMONIC_SKUP:
            return append_x(OPCODE_SKUP, v[0].value);

        case MNEMONIC_GDELAY:
            return append_x(OPCODE_GDELAY, v[0].value);

        case MNEMONIC_SDELAY:
            return append_x(OPCODE_SDELAY, v[0].value);

        case MNEMONIC_SSOUND:
            return append_x(OPCODE_SSOUND, v[0].value);

        case MNEMONIC_ADI:
            return append_x(OPCODE_ADI, v[0].value);

        case MNEMONIC_KEY:
            return append_x(OPCODE_KEY, v[0].value);

        case MNEMONIC_FONT:
            return append_x(OPCODE_FONT, v[0].value);

        case MNEMONIC_BCD:
            return append_x(OPCODE_BCD, v[0].value);

        case MNEMONIC_STR:
            return append_x(OPCODE_STR, v[0].value);

        case MNEMONIC_LDR:
            return append_x(OPCODE_LDR, v[0].value);

        case MNEMONIC_SKEQ:
        case MNEMONIC_SKNE:
        case MNEMONIC_MOV:
        case MNEMONIC_ADD:
            return get_word_dual_op(type, v);

        default:
            assert(0 && "invalid mnemonic");
    }
}

static uint16_t resolve_value(const SymbolTable *table, const Value value) {
    uint16_t result;
    assert(symbol_resolve_value(table, value, &result));
    return result;
}

void cg_emit_stmt(SymbolTable *table, Stmt stmt, ByteBuffer *out) {
    switch (stmt.type) {
        case STATEMENT_INSTR: {
            InstrStmt instr = stmt.instr;
            ROP v[instr.op_count];
    
            for (size_t o = 0; o < instr.op_count; o++) {
                Operand op = instr.operands[o];
                
                switch (op.type) {
                    case OPERAND_REG:
                        v[o] = (ROP){.type=op.type, .value=op.reg};
                        break;
                    case OPERAND_VALUE:
                        v[o] = (ROP){.type=op.type, .value=resolve_value(table, op.value)};
                        break;
                }
            }
    
            buf_write_u16(out, get_word(instr.type, v));
            break;
        }

        case STATEMENT_DIRECTIVE: {
            switch (stmt.drt.type) {
                case DIRECTIVE_DB: {
                    for (size_t i = 0; i < stmt.drt.db.count; i++) {
                        buf_write_u8(out, stmt.drt.db.bytes[i]);
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case STATEMENT_LABEL:
            /* does nothing */
            break;
    }
}
