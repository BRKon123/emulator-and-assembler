#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "instructionFormats.h"

//InstructionGroup struct and is_in_instruction_group() fn moved to instructionFormats.h for use in other files
struct InstructionGroup data_processing_group = {
    .instructions = (char* []){
        "add", "adds", "sub", "subs",
        "cmp", "cmn",
        "neg", "negs",
        "and", "ands", "bic", "bics",
        "eor", "orr", "eon", "orn",
        "tst",
        "movk", "movn", "movz",
        "mov",
        "mvn",
        "madd", "msub",
        "mul", "mneg"
    },
    .instruction_count = 26
};

struct InstructionGroup branch_group = {
   .instructions = (char* []){
    "b",
    "br",
    "b.eq", "b.ne", "b.ge", "b.lt", "b.gt", "b.le", "b.al"
     },
    .instruction_count = 9
};

struct InstructionGroup single_data_transfer_group = {
    .instructions = (char* []){"ldr", "str"},
    .instruction_count = 2
};

struct InstructionGroup special_group = {
    .instructions = (char* []){"nop", ".int"},
    .instruction_count = 3
};

/**
 * Used to send an instruction to the correct file.
 * @param argc used to bound the number of arguments.
 * @param argv list of string arguments.
**/
int instruction_processor(int argc, char** argv) {
  char* mnemonic = argv[0];

  //HALT instruction is done via data_processing - as bits match directly

  if (strchr(mnemonic, ':') != NULL) {
    // Perform action for labels
    // return;
    // printf("Mnemonic '%s' is a label.\n", mnemonic);
    return 0;
  }

  if (is_in_instruction_group(mnemonic, &data_processing_group)) {
    // Perform action for data processing group
    // return data_processing(argc, argv);
    // printf("Mnemonic '%s' is in the data processing group.\n", mnemonic);
    return data_processing(argc, argv);
  }

  if (is_in_instruction_group(mnemonic, &branch_group)) {
    // Perform action for branch group
    // printf("Mnemonic '%s' is in the branch group.\n", mnemonic);
    return process_branch_instruction(argc, argv);
  }

  if (is_in_instruction_group(mnemonic, &single_data_transfer_group)) {
    // Perform action for single data transfer group
    // printf("Mnemonic '%s' is in the single data transfer group.\n", mnemonic);
    return process_sdp(argc, argv);
    // return;
  }

  if (is_in_instruction_group(mnemonic, &special_group)) {
    // Perform action for special group
    // printf("Mnemonic '%s' is in the special group.\n", mnemonic);
    return special_instruction_decoder(argc, argv);
  }

  // printf("Unrecognised mnemonic '%s'.\n", mnemonic);
  return 0; // Should not get here.
}

