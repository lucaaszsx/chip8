# CASM8 - CHIP-8 Assembler

## Table of Contents

- [Introduction](#introduction)
    - [Assembler Overview](#assembler-overview)
    - [Assembly](#assembly)
        - [General Syntax Rules](#general-syntax-rules)
        - [Notation](#notation)
        - [Labels](#labels)
        - [Directives](#directives)
        - [Instructions](#instructions)

## Introduction

### Assembler Overview

CASM8 is a simple assembler designed to convert assembly code into executable programs for interpreters and emulators compatible with the CHIP-8 VM. The language was designed to be small and simple while still being capable of providing all the necessary features for developing programs for CHIP-8.

### Assembly

The information provided below details specifications of the way the Assembly works that CASM8 is capable of reading and translating into machine code, detailing syntax, labels, instructions and directives (pseudo-instructions).

#### General Syntax Rules

- Every statement in the assembly language must be separated by line breaks, and a label, instruction or a directive is expected to fill the one line of text.
- A semicolon (`;`) indicates a comment, and a comment ends when a line break is found.
- Spaces and multiple lines do not affect the final output and can be used for indentation and code organization.
- A dot must always precede the name of the directives. E.g.: `.org 0x200`
- A colon must always follow an identifier to indicate a label. E.g.: `my_label:`
- An instruction does not necessarily need to be inside a label.

#### Notation

The following notations are used throughout the document.

| Notation   | Meaning                                                                          | Note |
| ---------- | -------------------------------------------------------------------------------- | - |
| `BYTE`     | A single byte (8-bit value).                                                     | ~ |
| `NIBBLE`   | A 4-bit immediate value (0x0-0xF).                                               | ~ |
| `VALUE`    | 12-bit immediate (0x000-0xFFF) or a reference to a label or a symbolic constant. | ~ |
| `ID`       | A identifier.                                                                    | Identifiers must start with a letter or an underscore and be followed by a sequence of alphanumeric characters or additional underscores. |
| `VX`, `VY` | One of the 16 general-purpose 8-bit registers, `V0` through `VF`.                | `X` and `Y` denote distinct register operand positions within an instruction; they do not refer to fixed registers. |
| `VF`       | The 16th GPR.                                                                    | Used internally as a flag by several instructions. Programs should avoid using it to store values of their own. |
| `I`        | The 12-bit address register.                                                     | ~ |

#### Labels

Labels are indicators for the assembler signaling that the first memory address associated with that label should be stored in the symbol table so it can be referenced later.

#### Directives

Directives are responsible for changing the default behavior of the assembler. Below is a table showing the directives along their syntax and description.

| Directive | Syntax                   | Description                                                                      | Note |
| --------- | ------------------------ | -------------------------------------------------------------------------------- | - |
| `org`     | `.org {VALUE}`           | Sets the source address. All subsequent code is assembled based on this address. | ~ |
| `db`      | `.db {BYTE}, [BYTE...]`  | Writes one or more bytes in memory.                                              | Must be inside a label to be referenced. Otherwise, it will be memory garbage that can't be accessed. |
| `equ`     | `.equ {ID} {VALUE}`      | Defines a symbolic constant that can be referenced throughout the program.       | ~ |
| `end`     | `.end`                   | Determines the end of the program. Nothing after this directive is read.         | ~ |

#### Instructions

Assembly instructions are those that will be converted into executable machine code. CASM8 supports a total of 29 instructions, which are converted into operations corresponding to the CHIP-8 ISA. Each instruction has its own operands. When multiple operands are required for a single instruction, the values must be separated by a comma. These instructions do not necessarily need to be placed inside a label, unless a part of the code needs to reference them.

##### Data Movement Instructions

| Mnemonic | Syntax             | Opcode | Description                                                             | Notes |
| -------- | ------------------ | ------ | ----------------------------------------------------------------------- | - |
| `MOV`    | `MOV {VX}, {VY}`   | `8XY0` | Sets `VX` to the value of `VY`.                                         | ~ |
| `MOV`    | `MOV {VX}, {BYTE}` | `6XKK` | Sets `VX` to `BYTE`.                                                    | ~ |
| `MVI`    | `MVI {VALUE}`      | `ANNN` | Sets `I` to `VALUE`.                                                    | ~ |
| `KEY`    | `KEY {VX}`         | `FX0A` | Waits for a key press and stores the key value in `VX`.                 | Execution halts until a key is pressed. |
| `GDELAY` | `GDELAY {VX}`      | `FX07` | Sets `VX` to the current value of the delay timer.                      | ~ |
| `SDELAY` | `SDELAY {VX}`      | `FX15` | Sets the delay timer to the value of `VX`.                              | ~ |
| `SSOUND` | `SSOUND {VX}`      | `FX18` | Sets the sound timer to the value of `VX`.                              | ~ |
| `FONT`   | `FONT {VX}`        | `FX29` | Sets `I` to the memory address of the hexadecimal font sprite for `VX`. | ~ |
| `STR`    | `STR {VX}`         | `FX55` | Stores `V0` through `VX` in memory, starting at the address in `I`.     | ~ |
| `LDR`    | `LDR {VX}`         | `FX65` | Loads `V0` through `VX` from memory, starting at the address in `I`.    | ~ |

##### Binary Arithmetic Instructions

| Mnemonic | Syntax             | Opcode | Description                | Notes |
| -------- | ------------------- | ------ | ------------------------- | - |
| `ADD`    | `ADD {VX}, {VY}`    | `8XY4` | Sets `VX` to `VX + VY`.   | `VF` is set to `1` if the result overflows 8 bits, `0` otherwise. |
| `ADD`    | `ADD {VX}, {BYTE}`  | `7XKK` | Sets `VX` to `VX + BYTE`. | `VF` is not affected. |
| `SUB`    | `SUB {VX}, {VY}`    | `8XY5` | Sets `VX` to `VX - VY`.   | `VF` is set to `1` if `VX > VY` (no borrow), `0` otherwise. |
| `ADI`    | `ADI {VX}`          | `FX1E` | Sets `I` to `I + VX`.     | ~ |

##### Logical Instructions

| Mnemonic | Syntax               | Opcode | Description                              | Notes |
| -------- | --------------------- | ------ | --------------------------------------- | - |
| `OR`     | `OR {VX}, {VY}`       | `8XY1` | Sets `VX` to `VX OR VY`.                | ~ |
| `AND`    | `AND {VX}, {VY}`      | `8XY2` | Sets `VX` to `VX AND VY`.               | ~ |
| `XOR`    | `XOR {VX}, {VY}`      | `8XY3` | Sets `VX` to `VX XOR VY`.               | ~ |
| `SHR`    | `SHR {VX}`            | `8XY6` | Shifts `VX` right by one bit.           | `VF` is set to the least significant bit of `VX` prior to the shift. |
| `SHL`    | `SHL {VX}`            | `8XYE` | Shifts `VX` left by one bit.            | `VF` is set to the most significant bit of `VX` prior to the shift. |

##### Control Transfer Instructions

| Mnemonic | Syntax              | Opcode | Description                                  | Notes |
| -------- | -------------------- | ------ | ------------------------------------------- | - |
| `JMP`    | `JMP {VALUE}`        | `1NNN` | Sets the program counter to `VALUE`.        | ~ |
| `JMI`    | `JMI {VALUE}`        | `BNNN` | Sets the program counter to `VALUE + V0`.   | ~ |
| `JSR`    | `JSR {VALUE}`        | `2NNN` | Calls the subroutine at `VALUE`.            | Pushes the current address onto the stack before jumping. |
| `RTS`    | `RTS`                | `00EE` | Returns from the current subroutine.        | Pops the return address off the stack. |
| `SKEQ`   | `SKEQ {VX}, {VY}`    | `5XY0` | Skips the next instruction if `VX = VY`.    | ~ |
| `SKEQ`   | `SKEQ {VX}, {BYTE}`  | `3XKK` | Skips the next instruction if `VX = BYTE`.  | ~ |
| `SKNE`   | `SKNE {VX}, {VY}`    | `9XY0` | Skips the next instruction if `VX != VY`.   | ~ |
| `SKNE`   | `SKNE {VX}, {BYTE}`  | `4XKK` | Skips the next instruction if `VX != BYTE`. | ~ |

##### I/O Instructions

| Mnemonic | Syntax                      | Opcode | Description                                                                            | Notes |
| -------- | --------------------------- | ------ | -------------------------------------------------------------------------------------- | - |
| `CLS`    | `CLS`                       | `00E0` | Clears the display.                                                                    | ~ |
| `DRAW`   | `DRAW {VX}, {VY}, {NIBBLE}` | `DXYN` | Draws an `NIBBLE`-byte sprite from memory at `I` at position `(VX, VY)`.               | `VF` is set to `1` on pixel collision, `0` otherwise. Sprites wrap around screen edges. |
| `SKPR`   | `SKPR {VX}`                 | `EX9E` | Skips the next instruction if the key with the value of `VX` is currently pressed.     | ~ |
| `SKUP`   | `SKUP {VX}`                 | `EXA1` | Skips the next instruction if the key with the value of `VX` is currently not pressed. | ~ |

##### Miscellaneous Instructions

| Mnemonic | Syntax                | Opcode | Description                                                               | Notes |
| -------- | --------------------- | ------ | ------------------------------------------------------------------------- | - |
| `RAND`   | `RAND {VX}, {BYTE}`   | `CXKK` | Sets `VX` to a random byte ANDed with `BYTE`.                             | ~ |
| `BCD`    | `BCD {VX}`            | `FX33` | Stores the BCD representation of `VX` in memory at `I`, `I+1`, and `I+2`. | ~ |
