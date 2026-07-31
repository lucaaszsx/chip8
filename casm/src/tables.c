#include "tables.h"
#include "ast.h"
#include "util.h"
#include <string.h>

/* Directives table */
const DirectiveEntry drt_table[] = {
    {"ORG", DIRECTIVE_ORG},
    {"DB", DIRECTIVE_DB},
    {"EQU", DIRECTIVE_EQU},
    {"END", DIRECTIVE_END},
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
    {"CLS", MNEMONIC_CLS, {0}, 0},
    {"RTS", MNEMONIC_RTS, {0}, 0},
    {"JMP", MNEMONIC_JMP, {OPKIND_ADDR}, 1},
    {"JSR", MNEMONIC_JSR, {OPKIND_ADDR}, 1},
    {"SKEQ", MNEMONIC_SKEQ, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"SKNE", MNEMONIC_SKNE, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"MOV", MNEMONIC_MOV, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"ADD", MNEMONIC_ADD, {OPKIND_REG, OPKIND_REG | OPKIND_BYTE}, 2},
    {"SUB", MNEMONIC_SUB, {OPKIND_REG, OPKIND_REG}, 2},
    {"RSB", MNEMONIC_RSB, {OPKIND_REG, OPKIND_REG}, 2},
    {"OR", MNEMONIC_OR, {OPKIND_REG, OPKIND_REG}, 2},
    {"AND", MNEMONIC_AND, {OPKIND_REG, OPKIND_REG}, 2},
    {"XOR", MNEMONIC_XOR, {OPKIND_REG, OPKIND_REG}, 2},
    {"SHR", MNEMONIC_SHR, {OPKIND_REG}, 1},
    {"SHL", MNEMONIC_SHL, {OPKIND_REG}, 1},
    {"MVI", MNEMONIC_MVI, {OPKIND_ADDR}, 1},
    {"JMI", MNEMONIC_JMI, {OPKIND_REG, OPKIND_BYTE}, 2},
    {"RAND", MNEMONIC_RAND, {OPKIND_REG, OPKIND_BYTE}, 2},
    {"DRAW", MNEMONIC_DRAW, {OPKIND_REG, OPKIND_REG, OPKIND_NIBBLE}, 3},
    {"SKPR", MNEMONIC_SKPR, {OPKIND_REG}, 1},
    {"SKUP", MNEMONIC_SKUP, {OPKIND_REG}, 1},
    {"GDELAY", MNEMONIC_GDELAY, {OPKIND_REG}, 1},
    {"SDELAY", MNEMONIC_SDELAY, {OPKIND_REG}, 1},
    {"SSOUND", MNEMONIC_SSOUND, {OPKIND_REG}, 1},
    {"ADI", MNEMONIC_ADI, {OPKIND_REG}, 1},
    {"KEY", MNEMONIC_KEY, {OPKIND_REG}, 1},
    {"FONT", MNEMONIC_FONT, {OPKIND_REG}, 1},
    {"BCD", MNEMONIC_BCD, {OPKIND_REG}, 1},
    {"STR", MNEMONIC_STR, {OPKIND_REG}, 1},
    {"LDR", MNEMONIC_LDR, {OPKIND_REG}, 1}
};

/* number of mnemonics */
#define NUM_MNEMONICS (sizeof(mnemonic_table) / sizeof(mnemonic_table[0]))

bool get_mnemonic(char *s, MnemonicType type, MnemonicEntry *out) {
    for (size_t k = 0; k < NUM_MNEMONICS; k++) {
        MnemonicEntry entry = mnemonic_table[k];
        if (
            (s != NULL && istrcasecmp(entry.name, s) == 0) ||
            (s == NULL && entry.type == type)
        ) {
            *out = entry;
            return true;
        }
    }
    return false;
}
