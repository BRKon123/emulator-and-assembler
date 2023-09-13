#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashTable.h"
#include "filehandling.h"
#include "assembler_utilities.h"
#include "instructionFormats.h"
#include "sdpinstructions.h"

/* This function processes single data transfer instructions.
   It checks the number of tokens to understand what kind of instruction it is.
   It assumes that the assembly has been written correctly and that the tokenizer and
   instruction handler are also correct. */
int32_t process_sdp(int argc, char** argv) {
  char* rtName = argv[1]; // 1st arg is register name
  int32_t signFlag = (rtName[0] == 'x') ? 1 : 0;
  // Remove first char by shifting pointer, then find index of the register
  int32_t rtIndex = string_to_number(++rtName);

  if (argv[2][0] != '[') {
    // This must be a load literal
    char* literal = argv[2];
    int* literalAddress = 0;
    if (literal[0] == '#') {
      // This is an immediate
      literal++; // Add 1 to pointer to skip the first char (#)
    }
    else {
      // Literal is a label, so search symbol table
      literalAddress = (int*)search_hash_table(symbolTable, literal);
    }
    if (literalAddress == NULL) printf("\nWAITWAITWAIT %s\n", literal);
    return create_load_literal(signFlag, rtIndex, *literalAddress / 4);
  }
  else {
    // Either pre/post index, unsigned offset, or register offset
    char xnName[MAX_STR_LEN]; // Need to be able to write to this string
    strcpy(xnName, argv[2]);
    int32_t loadOrStore = (argv[0][0] == 'l') ? 1 : 0;
    memmove(xnName, xnName + 1, strlen(xnName)); // Remove the leading '['
    int32_t xnIndex;
    int32_t xmIndex;

    if(argc < 4){
      argv[3] = strdup("#0]");
      ALLOC_EXCEPTION(argv[3]);
    }


    if (argv[3][0] == '#') {
      // Either pre/post index or unsigned offset
      char immName[MAX_STR_LEN];
      strcpy(immName, argv[3]);
      memmove(immName, immName + 1, strlen(immName)); // Remove the leading '#'
      char immNameLastChar = immName[strlen(immName) - 1];

      if (immNameLastChar == '!') {
        // Pre index
        immName[strlen(immName) - 2] = '\0'; // Remove trailing ']!'
        if (strcmp(xnName, "SP") == 0) {
          xnIndex = 31;
        }
        else {
          memmove(xnName, xnName + 1, strlen(xnName)); // Remove the leading '['
          xnIndex = string_to_number(xnName);
        }
        return create_pre_post_index(loadOrStore, signFlag, rtIndex, string_to_number(immName), 1, xnIndex);
      }
      else if (immNameLastChar == ']') {
        // Unsigned offset
        immName[strlen(immName) - 1] = '\0'; // Remove trailing ']'
        if (strcmp(xnName, "SP") == 0) {
          xnIndex = 31;
        }
        else {
          memmove(xnName, xnName + 1, strlen(xnName)); // Remove the leading '['
          xnIndex = string_to_number(xnName);
        }
        return create_unsigned_offset(loadOrStore, signFlag, rtIndex, string_to_number(immName), xnIndex);
      }
      else {
        // Post index
        xnName[strlen(xnName) - 1] = '\0'; // Remove the trailing ']'
        if (strcmp(xnName, "SP") == 0) {
          xnIndex = 31;
        }
        else {
          memmove(xnName, xnName + 1, strlen(xnName)); // Remove the leading '['
          xnIndex = string_to_number(xnName);
        }
        return create_pre_post_index(loadOrStore, signFlag, rtIndex, string_to_number(immName), 0, xnIndex);
      }
    }
    else {
      // Register offset
      char xmName[MAX_STR_LEN];
      strcpy(xmName, argv[3]);
      xmName[strlen(xmName) - 1] = '\0'; // Remove the trailing ']'
      if (strcmp(xnName, "SP") == 0) {
        xnIndex = 31;
      }
      else {
        memmove(xnName, xnName + 1, strlen(xnName)); // Remove the leading '['
        xnIndex = string_to_number(xnName);
      }
      if (strcmp(xmName, "SP") == 0) {
        xmIndex = 31;
      }
      else {
        memmove(xmName, xmName + 1, strlen(xmName)); // Remove the leading '['
        xmIndex = string_to_number(xmName);
      }
      return create_register_offset(loadOrStore, signFlag, rtIndex, xmIndex, xnIndex);
    }
  }
  return 0;
}

int32_t create_load_literal(int32_t signFlag, int32_t rt, int32_t literal) {
  ll myInstr = init_load_literal();
  myInstr.sf = signFlag;
  myInstr.simm19 = literal - (currentAddress / 4);
  myInstr.rt = rt;

  instrUnion myUnion;
  myUnion.llBits = myInstr;
  return myUnion.value;
}

int32_t create_register_offset(int32_t loadOrStore, int32_t signFlag, int32_t rt, int32_t xm, int32_t xn) {
  sdt myInstr = init_sdt();
  myInstr.sf = signFlag;
  myInstr.rt = rt;
  myInstr.xn = xn;
  myInstr.l = loadOrStore;
  myInstr.u = 0;
  myInstr.offset = (xm << 6) | REG_OFFSET_PREDEFINED;

  instrUnion myUnion;
  myUnion.sdtBits = myInstr;
  return myUnion.value;
}

int32_t create_pre_post_index(int32_t loadOrStore, int32_t signFlag, int32_t rt, int32_t simm9, int32_t preOrPost, int32_t xn) {
  sdt myInstr = init_sdt();
  myInstr.sf = signFlag;
  myInstr.rt = rt;
  myInstr.xn = xn;
  myInstr.l = loadOrStore;
  myInstr.u = 0;
  myInstr.offset = ((simm9 & 0x1FF) << 2) | (preOrPost << 1) | 1;

  instrUnion myUnion;
  myUnion.sdtBits = myInstr;
  return myUnion.value;
}

int32_t create_unsigned_offset(int32_t loadOrStore, int32_t signFlag, int32_t rt, int32_t imm12, int32_t xn) {
  sdt myInstr = init_sdt();
  myInstr.sf = signFlag;
  myInstr.rt = rt;
  myInstr.xn = xn;
  myInstr.l = loadOrStore;
  myInstr.u = 1;
  myInstr.offset = imm12 / 8;

  instrUnion myUnion;
  myUnion.sdtBits = myInstr;
  return myUnion.value;
}