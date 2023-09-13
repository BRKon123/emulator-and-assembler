#ifndef DATA_PROC_ASSEMBLER_H
#define DATA_PROC_ASSEMBLER_H

#include <string.h>
#include "instructionFormats.h"
#include <stdio.h>
#include "assembler_utilities.h"

// Set the Sign Flag (SF) based on the input string
#define SET_SF(s) (s[0] == 'x' ? 1 : 0)

// Throw an exception when the mnemonic is not recognized or correctly handled
#define THROW_MNEMONIC_NOT_RECOGNISED illegal_argument_exception("Mnemonic not recognised/correctly handled")

// Compare two strings and check if they are equal
#define COMPARESTR(x, y) strcmp(x, y) == 0

// Multiplier used for wide move and shift operations
#define WIDE_MOVE_SHIFT_MULTIPLIER 16

// Instruction group for multiplications
#define MULTIPLICATIONS (InstructionGroup) {.instructions = (char * []) {"mul", "mneg", "madd", "msub"}, .instruction_count = 4}

// Instruction group for two-operand arithmetic operations
#define ARITHMETIC_TWOOP (InstructionGroup) {\
  .instructions = (char * []) {"add", "adds", "sub", "subs"},\
  .instruction_count = 4\
}

// Instruction group for two-operand logical operations
#define LOGICAL_TWOOP (InstructionGroup) {\
  .instructions = (char * []) {"and", "ands", "bic", "bics", "eor", "eon", "orr", "orn"},\
  .instruction_count = 8\
}

// Instruction group for wide moves
#define WIDE_MOVES (InstructionGroup) {\
  .instructions = (char * []) {"movz", "movk", "movn"},\
  .instruction_count = 3\
}

// Instruction group for aliases
#define ALIASES(InstructionGroup) {\
  .instructions = (char * []) {"mov", "neg", "negs", "mvn", "cmp", "cmn", "tst"},\
  .instruction_count = 7\
}

// Enumeration for different shift types
typedef enum {
  LSL = 0,
  LSR = 1,
  ASR = 2,
  ROR = 3
} shifts;

// Enumeration for different data processing types
typedef enum {
  REG,
  IMM
} dpType;

// Structure to hold shift information
typedef struct shift_info {
  shifts shift_t;
  uint16_t shiftBits : 12;
} shift_info;

#endif
