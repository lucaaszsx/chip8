#include "tables.h"
#include "ast.h"
#include "util.h"
#include <string.h>

/* Directives table */
const DirectiveEntry drt_table[] = {
    {"org", DIRECTIVE_ORG},
    {"db", DIRECTIVE_DB},
    {"equ", DIRECTIVE_EQU},
    {"end", DIRECTIVE_END},
};

/* number of directives */
#define NUM_DIRECTIVES (sizeof(drt_table) / sizeof(drt_table[0]))

DirectiveType get_directive_type(char *s) {
    for (size_t k = 0; k < NUM_DIRECTIVES; k++) {
        if (istrcasecmp(drt_table[k].name, s) == 0)
            return drt_table[k].type;
    }
    return DIRECTIVE_UNKNOWN;
}

/* Mnemonics table */
const MnemonicEntry mnemonic_table[] = {
    {"cls", MNEMONIC_CLS, {0}, 0},
    {"rts", MNEMONIC_RTS, {0}, 0},
    {"jmp", MNEMONIC_JMP, {OPKIND_ADDR}, 1},
    {"jsr", MNEMONIC_JSR, {OPKIND_ADDR}, 1},
    {"skeq", MNEMONIC_SKEQ, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"skne", MNEMONIC_SKNE, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"mov", MNEMONIC_MOV, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"add", MNEMONIC_ADD, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"sub", MNEMONIC_SUB, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"or", MNEMONIC_OR, {OPKIND_REG, OPKIND_REG}, 2},
    {"and", MNEMONIC_AND, {OPKIND_REG, OPKIND_REG}, 2},
    {"xor", MNEMONIC_XOR, {OPKIND_REG, OPKIND_REG}, 2},
    {"shr", MNEMONIC_SHR, {OPKIND_REG}, 1},
    {"shl", MNEMONIC_SHL, {OPKIND_REG}, 1},
    {"mvi", MNEMONIC_MVI, {OPKIND_ADDR}, 1},
    {"jmi", MNEMONIC_JMI, {OPKIND_REG, OPKIND_BYTE}, 2},
    {"rand", MNEMONIC_RAND, {OPKIND_REG, OPKIND_BYTE}, 2},
    {"draw", MNEMONIC_DRAW, {OPKIND_REG, OPKIND_REG, OPKIND_NIBBLE}, 3},
    {"skpr", MNEMONIC_SKPR, {OPKIND_REG}, 1},
    {"skup", MNEMONIC_SKUP, {OPKIND_REG}, 1},
    {"gdelay", MNEMONIC_GDELAY, {OPKIND_REG}, 1},
    {"sdelay", MNEMONIC_SDELAY, {OPKIND_REG}, 1},
    {"ssound", MNEMONIC_SSOUND, {OPKIND_REG}, 1},
    {"adi", MNEMONIC_ADI, {OPKIND_REG}, 1},
    {"key", MNEMONIC_KEY, {OPKIND_REG}, 1},
    {"font", MNEMONIC_FONT, {OPKIND_REG}, 1},
    {"bcd", MNEMONIC_BCD, {OPKIND_REG}, 1},
    {"str", MNEMONIC_STR, {OPKIND_REG}, 1},
    {"ldr", MNEMONIC_LDR, {OPKIND_REG}, 1}
};

/* number of mnemonics */
#define NUM_MNEMONICS (sizeof(mnemonic_table) / sizeof(mnemonic_table[0]))

bool get_mnemonic(char *s, MnemonicType type, MnemonicEntry *out) {
    for (size_t k = 0; k < NUM_MNEMONICS; k++) {
        MnemonicEntry entry = mnemonic_table[k];
        if (
            (s != NULL && strcmp(entry.name, s) == 0) ||
            (s == NULL && entry.type == type)
        ) {
            *out = entry;
            return true;
        }
    }
    return false;
}
