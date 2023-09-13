#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include <stdint.h>
#include <assert.h>
#include "instructionFormats.h"

// Maximum index of a register
#define MAX_REGISTER_INDEX 30

// Valid characters for a register name
#define VALID_REGISTER_CHARACTERS "xw"

// Suffix for the zero register
#define ZERO_REG_SUFFIX "zr"

// Number of bits in an instruction
#define NUM_INSTRUCTION_BITS 31

// Macro to check for allocation errors
#define ALLOC_EXCEPTION(ptr) \
  if (ptr == NULL) { \
    fprintf(stderr, "Alloc Error: variable alloc'd but no space found on the heap"); \
    exit(EXIT_FAILURE); \
  }

extern int32_t string_to_number(char* src);
extern void illegal_argument_number_exception(void);
extern void illegal_argument_exception(char* errorMessage);
extern void* safe_malloc(size_t size);
extern uint32_t get_register_index(char* registerName);
extern int32_t set_bit(int32_t bits, int8_t modifiedIndex, bool val);
extern uint32_t get_imm_value(char* value);
extern void illegal_argument_exception(char* errorMessage);

#endif