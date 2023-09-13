#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "instructionFormats.h"

// The following 5 functions initialise constant values in 
// instructions.
dpi init_dpi() {
  dpi myDPI;
  myDPI.op0 = DPI_OP0;
  return myDPI;
}

dpr init_dpr() {
  dpr myDPR;
  myDPR.op0 = DPR_OP0;
  return myDPR;
}

sdt init_sdt() {
  sdt mySDT;
  mySDT.msb = 0x1;
  mySDT.predefined1 = 0x0;
  mySDT.predefined2 = 0x1C;
  return mySDT;
}

ll init_load_literal() {
  ll myLL;
  myLL.msb = 0x0;
  myLL.predefined = 0x18;
  return myLL;
}

branch init_branch() {
  branch myBranch;
  myBranch.predefined = 0x5;
  myBranch.dontcares = 0;
  myBranch.operand = 0;
  return myBranch;
}

bool test_instruction_format() {
  dpr myInstr = init_dpr();
  myInstr.sf = 0x1;
  myInstr.opc = 0x2;
  myInstr.m = 0x0;
  myInstr.opr = 0x8;
  myInstr.rm = 0x4;
  myInstr.operand = 0x0;
  myInstr.rn = 0x2;
  myInstr.rd = 0x0;
  instrUnion myUnion;
  myUnion.dprBits = myInstr;
  printf("%0x", myUnion.value);
  return myUnion.value == 0xC6804000;
}

/**
 * @param mnemonic String of the instructions mnemonic (1st arg)
 * @param group the Instruction group of instructions.
 * **/
bool is_in_instruction_group(const char* mnemonic, InstructionGroup* group) {
  for (int i = 0; i < group->instruction_count; i++) {
    if (strcmp(mnemonic, group->instructions[i]) == 0) {
      return true;
    }
  }
  return false;
}
