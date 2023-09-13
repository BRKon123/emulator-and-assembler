#include "utilities.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>

#define UNCOND 0x00000000
#define REG 0x00000003
#define COND 0x00000001
#define EQ_PATTERN 0x00000000
#define NE_PATTERN 0x00000001
#define GE_PATTERN 0x0000000A
#define LT_PATTERN 0x0000000B
#define GT_PATTERN 0x0000000C
#define LE_PATTERN 0x0000000D
#define AL_PATTERN 0x0000000E

static void uncondBranch(int32_t simm26);
static void regBranch(uint32_t xn);
static void condBranch(int32_t simm19, uint32_t condType);

//function to decode a branch instruction into unconditional, register, or conditional
void decodeBranchInstruction(uint32_t instruction) {
  uint32_t branchType = bitMask(instruction, 30, 2);
  if (branchType == UNCOND) {
    uncondBranch(signedMask(instruction, 0, 26));
  } else if (branchType == REG) {
    regBranch(bitMask(instruction, 5, 5));
  } else if (branchType == COND) {
    condBranch(signedMask(instruction, 5, 19), bitMask(instruction, 0, 4));
  } else {
    fprintf(stderr, "Undefined instruction");
  }
}

//unconditionally increases program counter by given offset
static void uncondBranch(int32_t simm26) {
  pCounter += (simm26 * 4) - 4;
}

//unconditionally sets program counter to destination stored in register xn
static void regBranch(uint32_t xn) {
  pCounter = genRegisters[xn] - 4;
}

//condtionally increases the program counter
static void condBranch(int32_t simm19, uint32_t condType) {
  bool flag = false;
  flag = flag || (condType == EQ_PATTERN && psregisters.zero);
  flag = flag || (condType == NE_PATTERN && !psregisters.zero);
  flag = flag || (condType == GE_PATTERN && psregisters.neg == psregisters.overflow);
  flag = flag || (condType == LT_PATTERN && psregisters.neg != psregisters.overflow);
  flag = flag || (condType == GT_PATTERN && !psregisters.zero && psregisters.neg == psregisters.overflow);
  flag = flag || (condType == LE_PATTERN && !(!psregisters.zero && psregisters.neg == psregisters.overflow));
  flag = flag || (condType == AL_PATTERN);
  if (flag) {
    pCounter += (simm19 << 2) - 4;
  }
}