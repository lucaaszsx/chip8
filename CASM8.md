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

| Notation | Meaning  | Note |
| --- | --- | --- |
| `BYTE`   | A single byte (8-bit value).                                                     | ~ |
| `VALUE`  | 12-bit immediate (0x000-0xFFF) or a reference to a label or a symbolic constant. | ~ |
| `ID`     | A identifier.                                                                    | Identifiers must start with a letter or an underscore and be followed by a sequence of alphanumeric characters or additional underscores. |

#### Labels

Labels are indicators for the assembler signaling that the first memory address associated with that label should be stored in the symbol table so it can be referenced later.

#### Directives

Directives are responsible for changing the default behavior of the assembler. Below is a table showing the directives along their syntax and description.

| Directive | Syntax                   | Description                                                                      | Note                                     |
| --------- | ------------------------ | -------------------------------------------------------------------------------- | ---------------------------------------- |
| `org`     | `.org {VALUE}`           | Sets the source address. All subsequent code is assembled based on this address. | ~ |
| `db`      | `.db {BYTE}, [BYTE...]`  | Writes one or more bytes in memory.                                              | Must be inside a label to be referenced. Otherwise, it will be memory garbage that can't be accessed. |
| `equ`     | `.equ {ID} {VALUE}`      | Defines a symbolic constant that can be referenced throughout the program.       | ~ |
| `end`     | `.end`                   | Determines the end of the program. Nothing after this directive is read.         | ~ |

#### Instructions

Assembly instructions are those that will be converted into executable machine code. CASM8 supports a total of 29 instructions, which are converted into operations corresponding to the CHIP-8 ISA. Each instruction has its own operands. When multiple operands are required for a single instruction, the values must be separated by a comma. These instructions do not necessarily need to be placed inside a label, unless a part of the code needs to reference them.

##### Data Movement Instructions

##### Binary Arithmetic Instructions

##### Logical Instructions

##### Control Transfer Instructions

##### I/O Instructions
