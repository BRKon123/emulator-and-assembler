#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include "data.h"
#include "utilities.h"

#define add 0x0
#define sub 0x2
#define ZERO 0x1F
#define MSB_INDEX_64 63
#define MSB_INDEX_32 31

static int64_t addOrSubtract(int64_t r1, int64_t r2);
static int64_t regShift(void);
static int64_t computeBitwiseLogic(int64_t op2);
static void multiplyReg(void);
static void arithmeticReg(int64_t op2);
static void logicalReg(int64_t op2);

static bool sf;
static int32_t opc;
static int32_t opr;
static int64_t rm;
static int64_t operand;
static int64_t rn;
static int32_t rdIndex;

//Only care about bits 30 - 29 and 25 - 0
// PRE: We are passing in the correct instruction to this function -
// i.e. the instruction handler correctly executes this instruction
void registerDataProcessing(int32_t instr) {
  //initialising global variables
  sf      = getBit(instr, 31);
  opc     = bitMask(instr, 29, 2);
  opr     = bitMask(instr, 21, 4);
  rm      = bitMask(instr, 16, 5) == ZERO ? ZERO_R : genRegisters[bitMask(instr, 16, 5)];
  operand = bitMask(instr, 10, 6);
  rn      = bitMask(instr, 5, 5) == ZERO ? ZERO_R : genRegisters[bitMask(instr, 5, 5)];
  rdIndex = bitMask(instr, 0, 5);
  
  int32_t m = getBit(instr, 28);
  int64_t op2 = regShift();

  //If bit 4 of the opr is 1, then we have either a multiply or an arithmetic instruction
  if (opr == 0x8 && m) {
    multiplyReg();
  } else if (getBit(opr, 3) && !getBit(opr, 0) && !m) {
    arithmeticReg(op2);
  } else if (!getBit(opr, 3) && !m) {
    logicalReg(op2);
  } else {
    fprintf(stderr, "Undefined instruction\n");
  }
}

/*
Executes the multiply instruction for dpr
Modifies the value of the global variable 'opc'
*/
static void multiplyReg(void) {
  opc = getBit(operand, 5) ? sub : add; //getBit(operand, 5) represents 'x'.
  int64_t ra = bitMask(operand, 0, 5) == ZERO ? ZERO_R : genRegisters[bitMask(operand, 0, 5)];
  if (rdIndex == ZERO) {
    stackPtr = addOrSubtract(ra, rm * rn);
  } else {
    genRegisters[rdIndex] = addOrSubtract(ra, rm * rn);
  }
}

/* 
executes an arithmetic instruction
  @param 'op2' a 64-bit integer, after undergoing a logical or arithmetic shift
*/
static void arithmeticReg(int64_t op2) {
  if (rdIndex == ZERO) {
    stackPtr = addOrSubtract(rn, op2);
  } else {
    genRegisters[rdIndex] = addOrSubtract(rn, op2);
  }
}   

/* 
executes a bitwise logical instruction
  @param 'op2' a 64-bit integer, after undergoing a logical or arithmetic shift
*/
static void logicalReg(int64_t op2) {
  if (rdIndex == ZERO) {
    stackPtr = computeBitwiseLogic(op2);
  } else {
    genRegisters[rdIndex] = computeBitwiseLogic(op2);
  }
}

static int64_t addOrSubtract(int64_t r1, int64_t r2) {
  bool isSubtraction = getBit(opc, 1);
  bool flagsEnabled  = getBit(opc, 0);
  int64_t output     = isSubtraction ? r1 - r2 : r1 + r2;
  int msb = sf ? MSB_INDEX_64 : MSB_INDEX_32;
  int32_t msb_m = getBit(r1, msb);
  int32_t msb_n = getBit(r2, msb);
  output = sf ? output : output & 0xFFFFFFFF;
  int msb_out = getBit(output, msb);
  if (flagsEnabled) {
    psregisters.neg = msb_out;
    psregisters.zero = output == 0;
    if (!isSubtraction) {
  printf("%ld + %ld\n", r1, r2);
      psregisters.carry = ((msb_m | msb_n) && !msb_out) || (msb_m & msb_n & msb_out);
      psregisters.overflow = (msb_m == msb_n) && (msb_m != msb_out);
    } else {
  printf("%ld - %ld\n", r1, r2);
      psregisters.carry = ((uint64_t) r1) >= ((uint64_t) r2);
      psregisters.overflow = msb_m ^ msb_n && msb_n == msb_out;
    }
  }
  return output;
}


/*
  regShift() applies either a logical or arithmetic shift on a register
  @params opr - bits 21 - 24 of the instruction
  @params rm - register to be shifted
  @params shamt - shift amount - same as the operand for arithmetic/logical shifts
*/
static int64_t regShift() {
    int32_t shamt = operand;
    bool isArithmeticInstr = getBit(opr, 3);
    int32_t shiftType = bitMask(opr, 1, 2);
    int32_t n = getBit(opr, 0);
    int64_t op2;
    switch(shiftType) {
        case 0x0:
            op2 = logicalLeft(rm, sf, shamt);
            break;
        case 0x1:
            op2 = logicalRight(rm, sf, shamt);
            break;
        case 0x2:
            op2 = arithmeticRight(rm, sf, shamt);
            break;
        default:
            // assuming shift code of 11 means no shift for an arithmetic instruction
            op2 = isArithmeticInstr ? rm : rotateRight(rm, sf, shamt);
            printf("%lx\n", op2);
            break;
    }
    return n ? ~op2 : op2;
}

static int64_t computeBitwiseLogic(int64_t op2) {
    int64_t output;
    switch(opc) {
        case 0x0:
            output = rn & op2;
            break;
        case 0x1:
            output = rn | op2;
            break;
        case 0x2:
            output = rn ^ op2;
            break;
        default:
            output = rn & op2;
            output = sf ? output : output & 0xFFFFFFFF;
            psregisters.neg = sf ? getBit(output, MSB_INDEX_64) : getBit(output, MSB_INDEX_32);
            psregisters.zero = output == 0;
            psregisters.carry = 0;
            psregisters.overflow = 0;
            break;
    }
    return sf ? output : output & 0xFFFFFFFF;
}