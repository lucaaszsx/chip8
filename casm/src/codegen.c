#include <assert.h>
#include "codegen.h"

#define CG_BUFFER_ICAP 1024

void cg_init(CG *cg, SymbolTable *table) {
    buf_init(cg->buffer, CG_BUFFER_ICAP);

    cg->pc = 0;
    cg->table = table;
}

void cg_free(CG *cg) {
    buf_free(cg->buffer);

    cg->buffer = NULL;
    cg->table = NULL;
    cg->pc = 0;
}

static uint16_t resolve_value(const CG *cg, const Value value) {
    uint16_t result;
    assert(symbol_resolve_value(cg->table, value, &result));
    return result;
}

static void cg_emit_byte(CG *cg, uint8_t byte) {
    buf_write_u8(cg->buffer, byte);
}

static void cg_emit_word(CG *cg, uint16_t word) {
    buf_write_u16(cg->buffer, word);
}

void cg_emit_instr(CG *cg, InstrStmt instr) {
    
}
