#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "assembler_utilities.h"

/**
  string_to_number is used to convert a number in string format to an int32_t
  @param 'src' is the source string to be converted
  eg. "23" = 23, "0xabc7" = 0xabc7
**/
int32_t string_to_number(char* src) {
  return strtol(src, NULL, 0);
}

/**
  gets the register number from a string representation of the register.
  @param 'registerName' is the register we want to find the index of - the name must be valid - i.e. starts with an 'x' or a 'w' and must be following by an integer between 0 and 30 inclusive.
    E.g.
      get_register_index("x13") = 13
      get_register_index("w10") = 10
      get_register_index("a23") and get_register_index("w84"); will throw errors as these are not valid register names
**/
uint32_t get_register_index(char* registerName) {
  // The registerName should not have a null pointer.
  assert(registerName != NULL);

  char leadingChar = registerName[0];
  char* regNumber = ++registerName;

  // If the register number is 0, return the zero register.
  // If the register is not of a valid type, throw exception.
  // Otherwise return the number.
  if (strcmp(regNumber, ZERO_REG_SUFFIX) == 0) {
    return NUM_INSTRUCTION_BITS;
  }
  else if (strchr(VALID_REGISTER_CHARACTERS, leadingChar) == NULL ||
    atoi(regNumber) > MAX_REGISTER_INDEX || atoi(regNumber) < 0) {
    illegal_argument_exception("Illegal argument exception: Register name not recognised\n");
    exit(1);
  }
  else {
    return atoi(regNumber);
  }
}

/**
 * Parse the given immediate value and return it as an unsigned integer.
 * @param value A string representing the immediate value.
 * @return The parsed immediate value as an unsigned integer.
 */
uint32_t get_imm_value(char* value) {
  value++;
  int length = strlen(value);
  if (length > 2) {
    if (strncmp(value, "0x", 2) == 0) {
      return strtoul(value + 2, NULL, 16);
    }
  }
  return strtoul(value, NULL, 10);
}

/**
 * Set the bit at the given index in the given bit string to the given value.
 * @param bits An int32_t representing the bit string to modify.
 * @param modifiedIndex An int8_t representing the index of the bit to modify.
 * @param val A bool representing the value to set the bit to (0 or 1).
 * @return The modified bit string.
 */
int32_t set_bit(int32_t bits, int8_t modifiedIndex, bool val) {
    switch (val) {
      case false: 
        return bits & (-1 - (1 << modifiedIndex));
      case true:
        return bits | (1 << modifiedIndex);
    }
}

/**
 * Handle an illegal argument exception with the given error message.
 *
 * @param errorMessage A string representing the error message to display.
 */
void illegal_argument_exception(char *errorMessage) {
  fprintf(stderr, "%s", errorMessage);
  exit(EXIT_FAILURE);
}

/**
 * Allocate memory with error checking. If the allocation fails, print an error message and exit the program.
 *
 * @param size A size_t representing the size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
void *safe_malloc(size_t size) {
  void *ptr = malloc(size);
  ALLOC_EXCEPTION(ptr);
  return ptr;
}

/**
 * Handle an illegal number of arguments exception.
 */
void illegal_argument_number_exception() {
  illegal_argument_exception("Illegal Number of Arguments for Instruction");
}

// // For debugging purposes
// void print_instructions_in_group(InstructionGroup* instrs) {
//   for (int i = 0; i < instrs->instruction_count; i++) {
//     printf("%s\n", instrs->instructions[i]);
//   }
// }