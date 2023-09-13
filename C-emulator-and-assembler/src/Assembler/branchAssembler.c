#include <stdio.h>
#include "instructionFormats.h"
#include "assembler_utilities.h"
#include "filehandling.h"
#include "hashTable.h"

#define REGISTER_INSTRUCTION_OPERAND 0xD61F0000
#define CONDITIONAL_INSTRUCTION_OPERAND 0x54000000
#define COND_EQ 0x0
#define COND_NE 0x1
#define COND_GE 0xA
#define COND_LT 0xB
#define COND_GT 0xC
#define COND_LE 0xD
#define COND_AL 0xE

extern HashTable* symbolTable;
extern int currentAddress;

/**
 * Process the immediate value of a branch instruction.
 *
 * @param imm The immediate value as a string.
 * @return The processed immediate value as an integer.
 */
int process_imm_value(char* imm) {
  if (strncmp(imm, "0x", 2) == 0) {
    return string_to_number(imm);
  }

  if (imm[0] == '#') {
    imm++;
    return string_to_number(imm);
  }
  int* value = (int*)search_hash_table(symbolTable, imm);
  if (value != NULL) {
    return ((*value) - currentAddress);
  }

  return string_to_number(imm);
  exit(0);
}

/**
 * Get the condition code for a branch instruction.
 *
 * @param cond The condition code as a string.
 * @return The condition code as an integer.
 */
int get_cond(char* cond) {
  if (strcmp(cond, "b.eq") == 0) return COND_EQ;
  if (strcmp(cond, "b.ne") == 0) return COND_NE;
  if (strcmp(cond, "b.ge") == 0) return COND_GE;
  if (strcmp(cond, "b.lt") == 0) return COND_LT;
  if (strcmp(cond, "b.gt") == 0) return COND_GT;
  if (strcmp(cond, "b.le") == 0) return COND_LE;
  if (strcmp(cond, "b.al") == 0) return COND_AL;
  return 0;
}

/**
 * Construct the instruction for a conditional branch.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The constructed instruction.
 */
int conditional_branch(int argc, char** argv) {
  return get_cond(argv[0]) | (((process_imm_value(argv[1]) / 4) & 0x7FFFF) << 5) | CONDITIONAL_INSTRUCTION_OPERAND;
}

/**
 * Construct the instruction for an unconditional branch.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The constructed instruction.
 */
int unconditional_branch(int argc, char** argv) {
  return (process_imm_value(argv[1]) / 4) & 0x3FFFFFF;
}

/**
 * Construct the instruction for a register branch.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The constructed instruction.
 */
int register_branch(int argc, char** argv) {
  return REGISTER_INSTRUCTION_OPERAND | (get_register_index(argv[1]) << 5);
}

/**
 * Process a branch instruction.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The processed branch instruction.
 */
int process_branch_instruction(int argc, char** argv) {
  instrUnion instruction;
  instruction.branchBits = init_branch();

  if (strcmp(argv[0], "b") == 0) {
    instruction.value |= unconditional_branch(argc, argv);
  }
  else if (strcmp(argv[0], "br") == 0) {
    instruction.value |= register_branch(argc, argv);
  }
  else {
    instruction.value |= conditional_branch(argc, argv);
  }
  return instruction.value;
}
