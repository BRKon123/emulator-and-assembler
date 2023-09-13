#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "special_instructions.h"
#include "assembler_utilities.h"

//halt is handled with 'and'

/*
decodes the special instructions into no-operation(NOP) or directive
@param 'argc' is the number of arguments
@param 'argv' is a pointer to strings (the input arguments)
*/
int32_t special_instruction_decoder(int argc, char** argv) {
  char* type = argv[0];
  if (strcmp(type, NO_OPERATION) == 0) {
    return no_operation(argc, argv);
  }
  if (strcmp(type, DIRECTIVE) == 0) {
    return implement_directive(argc, argv);
  }
  return 0;
}

/*
returns the encoding of the no_operation instruction
@param 'argc' is the number of arguments
@param 'argv' is a pointer to strings (the input arguments)
*/
int32_t no_operation(int argc, char** argv) {
  if (argc != 1) {
    illegal_argument_number_exception();
  }
  return nop;
}

/*
returns the bit respresentation of the input .int directive
@param 'argc' is the number of arguments
@param 'argv' is a pointer to strings (the input arguments)
*/
int32_t implement_directive(int argc, char** argv) {
  return string_to_number(argv[1]);
}