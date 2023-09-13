#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include "data.h"
#include "utilities.h"


/*
Performs an arithmetic operation based on the given parameters.
@param sf The sign flag indicating whether the operation is 32/64.
@param opc The operation code specifying the type of arithmetic operation
  (subtraction or addition) and flags.
@param operand The operand value used in the arithmetic operation.
@param rd The destination register where the result will be stored.
*/
static void arithmetic_imm(int32_t sf, int32_t opc, int32_t operand, int32_t rd) {
  // Deconstruct the operand.
  int32_t sh = bitMask(operand, 17, 1);    // (operand & 0b100000000000000000) >> 17;
  int32_t imm12 = bitMask(operand, 5, 12); // (operand & 0b011111111111100000) >> 5;
  int32_t rn = bitMask(operand, 0, 5);     // (operand & 0b000000000000011111);

  // op2 is the right hand side of the opperation
  // Shift imm12 if sh is 1
  int64_t op2 = (sh) ? (imm12 << 12) : imm12;

  // Create a variable for the result of the calculation
  int64_t result = 0;

  // The conditional flags and operation (+) or (-).
  int8_t set_pstate = opc & 0x1; // 1 = yes, 0 = no;
  int8_t is_sub = opc & 0x2; // 1 = sub, 0 = add;

  // Get the value at the register rn. If rn = 11111 then use the stack pointer.
  int64_t rn_val = (rn == 0x1F) ? stackPtr : genRegisters[rn];

  if (!sf) {
    rn_val = rn_val & 0xFFFFFFFF;
  }

  // Ez Pz Lemon Squeezy
  if (is_sub) {
    result = rn_val - op2;
  }
  else {
    result = rn_val + op2;
  }

  result = (sf) ? result : result & 0xFFFFFFFF;

  // Set the flags if intended.
  if (set_pstate) {

    int8_t msb = sf ? 63 : 31;

    // Get the signs of the summands and the result
    int8_t rn_sign = getBit(rn_val, msb);
    int8_t op_sign = getBit(op2, msb);
    int8_t res_sign = getBit(result, msb);

    // if u dont get how this bit work ion kno for u
    psregisters.neg = res_sign;
    psregisters.zero = (result == 0);

    // Test for carry & overflow.
    // If the summands have the same sign and the result is different then an overflow has occured.
    if (!is_sub) {
      psregisters.overflow = ((rn_sign == op_sign) && (res_sign != rn_sign));
      psregisters.carry = (sf) ? (LLONG_MAX - op2 < rn_val) : (INT_MAX - op2 < rn_val);
    }
    else {
      psregisters.overflow = ((rn_sign != op_sign) && (res_sign == op_sign));
      psregisters.carry = ((uint64_t)rn_val) >= ((uint64_t)op2);
    }
  }

  // Create a pointer to the destination of the result.
  int64_t* destination = &genRegisters[rd];

  // If rd = 11111 set it evaluates to either the zero reg or the stack 
  // pointer dependant on set_pstate (USE NULL instead of zero reg cause its same thing)
  if ((rd == 0x1F)) {
    if (set_pstate) {
      return;
    }
    else {
      destination = &stackPtr;
    }
  }
  // Set the thang to the thang.
  *destination = result;
}

/*
Performs an wide_move operation based on the given parameters.
@param sf The sign flag indicating whether the operation is 32/64.
@param opc The operation code specifying the type of move operation
@param operand The operand value used in the arithmetic operation.
@param rd The destination register where the result will be stored.
*/
static void wide_move(int32_t sf, int32_t opc, int32_t operand, int32_t rd) {
  // Segment the operand
  int32_t hw = bitMask(operand, 16, 2);      // (operand & 0b110000000000000000) >> 16;
  int64_t imm16 = bitMask(operand, 0, 16);   // (operand & 0b001111111111111111);

  // Calculate the shift.
  int64_t op2 = imm16 << (hw * 16);

  // Create variables of the result and destination of the result.
  // int64_t *destination = &();
  int64_t result = 0;
  int64_t rd_val;
  
  switch (opc) {
    // movn
  case 0x0:
    result = ~(op2);
    break;
    // movz
  case 0x2:
    result = op2;
    break;
    // movk
  case 0x3:
    // "Inserts" imm16 into rd dependent of the shift.
    // First line gets either 32 or 64 bits of rd;
    rd_val = (sf) ? genRegisters[rd] : (genRegisters[rd] & 0xFFFFFFFF);
    // This is a bit of a mouth full.
    // (rd_val & ((0xFFFFFFFFFFFFFFFF) ^ (int64_t)(((int64_t)0xFFFF) << (16 * hw))))
    // This line sets 16 bits in rd_val to be 0 dependent on shift.
    // The or at the end inserts the imm16.
    // printf("%lx", genRegisters[rd]);
    result = (rd_val & ((0xFFFFFFFFFFFFFFFF) ^ (int64_t)(((int64_t)0xFFFF) << (16 * hw)))) | (imm16 << 16 * hw);
    break;
  default:
    // Not Documented.
    break;
  }

  // Assign result, and truncate if 32 bits.
  result = (sf) ? result : (result & 0xFFFFFFFF);

  // Set destination register to result.
  genRegisters[rd] = result;
}

/*
 immediate_data_processing processes instructions with the 100x op0.
 @param 'instruction' full 32bits of the instruction.
*/
void immediate_data_processing(int32_t instruction)
{
  int32_t rd = bitMask(instruction, 0, 5);          // (instruction & 0b00000000000000000000000000011111);
  int32_t operand = bitMask(instruction, 5, 18);   // (instruction & 0b00000000011111111111111111100000) >> 5;
  int32_t opi = bitMask(instruction, 23, 3);        // (instruction & 0b00000011100000000000000000000000) >> 23;
  int32_t opc = bitMask(instruction, 29, 2);        // (instruction & 0b01100000000000000000000000000000) >> 29;
  int32_t sf = bitMask(instruction, 31, 1);          // (instruction & 0b10000000000000000000000000000000) >> 31;

  // Note other cases of opi do not need to be handled (according to spec).
  switch (opi)
  {
  case 0x5:
    wide_move(sf, opc, operand, rd);
    break;
  case 0x2:
    arithmetic_imm(sf, opc, operand, rd);
    break;
  default:
    // NOT HANDLED
    break;
  }
}


