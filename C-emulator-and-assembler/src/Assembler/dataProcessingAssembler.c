#include "dataProcessingAssembler.h"
#include <stdlib.h>

static instrUnion *multiply(int argc, char **argv);
static dpType get_type(char *operand);
static instrUnion *two_operand_dest(int argc, char **argv);
static instrUnion *aliases(int argc, char **argv);
static instrUnion *wide_move_assembler(int argc, char **argv);

/**
 * Process a data processing instruction.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The processed data processing instruction.
 */
int32_t data_processing(int argc, char **argv) {
  char *mnemonic = argv[0];
  instrUnion *returnBits;
  if (is_in_instruction_group(mnemonic, &MULTIPLICATIONS)) {
    returnBits = multiply(argc, argv);
  } else if (is_in_instruction_group(mnemonic, &WIDE_MOVES)) {
    returnBits = wide_move_assembler(argc, argv);
  } else if (is_in_instruction_group(mnemonic, &ARITHMETIC_TWOOP) || is_in_instruction_group(mnemonic, &LOGICAL_TWOOP)) {
    returnBits = two_operand_dest(argc, argv);
  } else {
    returnBits = aliases(argc, argv);
  }
  int32_t instruction = returnBits->value;
  free(returnBits);
  return instruction;
}

/**
 * Get the shift information for a register or immediate value.
 *
 * @param operandFields An array of strings representing the operand fields.
 *                      The first string represents the value to be shifted,
 *                      the second string represents the type of shift,
 *                      and the third string represents the shift amount.
 * @return The shift information.
 */
static shift_info get_shift(char **operandFields) {
  shift_info shiftInfo;
  char *shift = operandFields[1];
  if (get_type(operandFields[0]) == REG) {
    if (COMPARESTR(shift, "lsl")) {
      shiftInfo.shift_t = LSL;
    } else if (COMPARESTR(shift, "lsr")) {
      shiftInfo.shift_t = LSR;
    } else if (COMPARESTR(shift, "asr")) {
      shiftInfo.shift_t = ASR;
    } else {
      shiftInfo.shift_t = ROR;
    }
    shiftInfo.shiftBits = get_imm_value(operandFields[2]);
  } else {
    shiftInfo.shift_t = get_imm_value(operandFields[2]) == 0 ? LSR : LSL;
    shiftInfo.shiftBits = get_imm_value(operandFields[0]);
  }
  return shiftInfo;
}

/**
 * Set the shift bits in the data processing instruction.
 *
 * @param otherBits The rest of the instruction, with the shift bits field initialized to random values.
 * @param transferType The type of transfer (immediate or register).
 * @param shiftInfo The shift information.
 */
static void set_shift_bits(instrUnion *otherBits, dpType transferType, shift_info *shiftInfo) {
  if (transferType == IMM) {
    otherBits->dpiBits.sh = shiftInfo->shift_t == LSL ? 1 : 0;
    otherBits->dpiBits.imm12 = shiftInfo->shiftBits;
  } else {
    otherBits->dprBits.operand = shiftInfo->shiftBits;
    otherBits->dprBits.opr = (shiftInfo->shift_t) << 1;
  }
}

/**
 * Process a two-operand data processing instruction.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The processed two-operand data processing instruction.
 */
static instrUnion *two_operand_dest(int argc, char **argv) {
  char *mnemonic = argv[0];
  instrUnion *bits = safe_malloc(sizeof(instrUnion));
  bits->dprBits.rd = get_register_index(argv[1]);
  bits->dprBits.rn = get_register_index(argv[2]);
  bits->dprBits.sf = SET_SF(argv[1]);
  dpType transferType = get_type(argv[3]);
  shift_info shiftInfo;
  if (argc > 4) {
    assert(argc == 6);
    shiftInfo = get_shift(argv + 3);
  } else {
    if (transferType == REG) {
      shiftInfo.shiftBits = 0;
      shiftInfo.shift_t = LSL;
    } else {
      shiftInfo.shift_t = LSR;
      shiftInfo.shiftBits = get_imm_value(argv[3]);
    }
  }
  set_shift_bits(bits, transferType, &shiftInfo);
  if (get_type(argv[3]) == IMM) {
    bits->dpiBits.opi = 0x2;
    bits->dpiBits.op0 = DPI_OP0;
  } else {
    bits->dprBits.rm = get_register_index(argv[3]);
    bits->dprBits.op0 = DPR_OP0;
    bits->dprBits.m = 0;
  }

  if (is_in_instruction_group(mnemonic, &ARITHMETIC_TWOOP) && transferType == REG) {
    bits->dprBits.opr = set_bit(bits->dprBits.opr, 3, 1);
  }
  if (COMPARESTR(mnemonic, "add") || COMPARESTR(mnemonic, "and") || COMPARESTR(mnemonic, "bic")) {
    bits->dprBits.opc = 0x0;
  } else if (COMPARESTR(mnemonic, "adds") || COMPARESTR(mnemonic, "orr") || COMPARESTR(mnemonic, "orn")) {
    bits->dprBits.opc = 0x1;
  } else if (COMPARESTR(mnemonic, "sub") || COMPARESTR(mnemonic, "eor") || COMPARESTR(mnemonic, "eon")) {
    bits->dprBits.opc = 0x2;
  } else {
    bits->dprBits.opc = 0x3;
  }

  if (COMPARESTR(mnemonic, "bic") || COMPARESTR(mnemonic, "orn") || COMPARESTR(mnemonic, "eon") || COMPARESTR(mnemonic, "bics")) {
    bits->dprBits.opr = set_bit(bits->dprBits.opr, 0, 1);
  }
  return bits;
}

/**
 * Process an instruction with aliases.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The processed instruction with aliases.
 */
static instrUnion *aliases(int argc, char **argv) {
  int newArgLen = argc + 1;
  char **newArgs = safe_malloc((newArgLen) * sizeof(char *));
  char *mnemonic = argv[0];
  int paramLocation = (COMPARESTR(mnemonic, "cmp") || COMPARESTR(mnemonic, "cmn") || COMPARESTR(mnemonic, "tst")) ? 0 : 1;
  for (int i = argc; i > paramLocation; i--) {
    newArgs[i + 1] = argv[i];
  }
  newArgs[paramLocation] = argv[paramLocation];
  newArgs[paramLocation + 1] = SET_SF(argv[1]) == 1 ? "xzr" : "wzr";
  if (COMPARESTR(mnemonic, "cmp")) {
    newArgs[0] = "subs";
  } else if (COMPARESTR(mnemonic, "cmn")) {
    newArgs[0] = "adds";
  } else if (COMPARESTR(mnemonic, "tst")) {
    newArgs[0] = "ands";
  } else if (COMPARESTR(mnemonic, "neg")) {
    newArgs[0] = "sub";
  } else if (COMPARESTR(mnemonic, "negs")) {
    newArgs[0] = "subs";
  } else if (COMPARESTR(mnemonic, "mov")) {
    newArgs[0] = "orr";
  } else if (COMPARESTR(mnemonic, "mvn")) {
    newArgs[0] = "orn";
  } else {
    THROW_MNEMONIC_NOT_RECOGNISED;
  }
  instrUnion *bits = two_operand_dest(newArgLen, newArgs);
  free(newArgs);
  return bits;
}

/**
 * Process a multiply instruction.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The processed multiply instruction.
 */
static instrUnion *multiply(int argc, char **argv) {
  dpr mul = init_dpr();
  char *mnemonic = argv[0];
  mul.rd = get_register_index(argv[1]);
  mul.rn = get_register_index(argv[2]);
  mul.rm = get_register_index(argv[3]);
  mul.opr = 0x8;
  mul.m = 1;
  mul.opc = 0;
  mul.sf = SET_SF(argv[1]);
  if (!strcmp(mnemonic, "mul")) {
    mul.operand = 0x1f;
  } else if (!strcmp(mnemonic, "mneg")) {
    mul.operand = 0x3f;
  } else {
    mul.operand = get_register_index(argv[4]);
    if (!strcmp(mnemonic, "msub")) {
      mul.operand = set_bit(mul.operand, 5, 1);
    }
  }
  instrUnion *dprToInt = safe_malloc(sizeof(instrUnion));
  dprToInt->dprBits = mul;
  return dprToInt;
}

/**
 * Get the type of an operand.
 *
 * @param operand The operand string.
 * @return The type of the operand.
 */
static dpType get_type(char *operand) {
  if (operand[0] == '#') {
    return IMM;
  } else {
    return REG;
  }
}

/**
 * Process a wide move instruction.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The processed wide move instruction.
 */
static instrUnion *wide_move_assembler(int argc, char **argv) {
  instrUnion *bits = safe_malloc(sizeof(instrUnion));
  char *mnemonic = argv[0];
  bits->wm.sf = SET_SF(argv[1]);
  bits->wm.op0 = DPI_OP0;
  bits->wm.rd = get_register_index(argv[1]);
  bits->wm.imm16 = get_imm_value(argv[2]);
  bits->wm.hw = get_imm_value(argv[4]) / WIDE_MOVE_SHIFT_MULTIPLIER;
  bits->wm.opi = WIDE_MOVE_OPI;
  if (COMPARESTR(mnemonic, "movn")) {
    bits->wm.opc = 0;
  } else if (COMPARESTR(mnemonic, "movz")) {
    bits->wm.opc = 2;
  } else {
    bits->wm.opc = 3;
  }
  return bits;
}
