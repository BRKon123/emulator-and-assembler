#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "emulate.h"

// Initalization of variables.
regSize genRegisters[NUM_REGISTERS];
regSize pCounter;
regSize stackPtr;
struct PSTATE psregisters;
const regSize ZERO_R = 0;
int64_t* memory;
bool HALTED = false;

int main(int argc, char** argv)
{
  // DO NOT CHANGE CALLOC LINE - MEMORY INITIALISATION DEPENDS ON CALLOC SETTING ALL BITS TO 0
  memory = calloc(MEMORY_SIZE, sizeof(int64_t));
  if (memory == NULL) {
    fprintf(stderr, "Calloc Error: No space on heap for memory");
    exit(EXIT_FAILURE);
  }
  int64_t* const memStart = memory;
  initialise_registers();

  // Load File into Memory
  loadFileIntoMemory(argc, argv);
  
  // run() Runs File from pc = 1;
  runEmulator();

  output_processor_state(memStart, argv[2]);
  free(memory);
  return EXIT_SUCCESS;
}

// Used to initialise all registers to 0
// registers is a global variable, and included in data.h, so fn has no arguments
static void initialise_registers()
{
  for (int i = 0; i < NUM_REGISTERS; i++)
  {
    genRegisters[i] = 0;
  }
  pCounter = 0;
  stackPtr = 0;
  psregisters.neg = 0;
  psregisters.zero = 1;
  psregisters.carry = 0;
  psregisters.overflow = 0;
}

static void runEmulator() {
  while (!HALTED) {
    size_t element_index = pCounter / 8;
    size_t offset = pCounter % 8;
    uint8_t* ptr = (uint8_t*)&memory[element_index];
    int32_t instruction = 0;
    for (size_t i = 0; i < sizeof(int32_t); i++) {
      instruction |= (int32_t)ptr[offset + i] << (8 * i);
    }
    runInstruction(instruction);
    if (!HALTED) {
      pCounter += 4;
    }
  }
}

static void runInstruction(int32_t instruction) {
  int8_t ooox = bitMask(instruction, 26, 3);
  int8_t xooo = bitMask(instruction, 25, 3);
  int8_t xoxo = bitMask(instruction, 25, 1) + (bitMask(instruction, 27, 1) << 1);

  if (instruction == HALT) {
    // printf("HALT\n");
    HALTED = true;
    return;
  }
  if (instruction == NOP) {
    // pCounter++;
    // printf("NOP\n");
    return;
  }
  if (ooox == 0x4) {
    // Immediate Data Processing
    // printf("Immediate Data Processing\n");
    immediate_data_processing(instruction);
    return;
  }
  if (ooox == 0x5) {
    // Branch
    // printf("Branch\n");
    // pCounter = genRegisters[28] - 4;
    // runInstruction(HALT);
    decodeBranchInstruction(instruction);
    return;
  }
  if (xooo == 0x5) {
    // Register Data Processing
    // printf("Register Data Processing\n");
    registerDataProcessing(instruction);
    return;
  }
  if (xoxo == 0x2) {
    // Load Store
    // printf("Load\n");
    decodeSingleDataTransferInstruction(instruction, memory);
    return;
  }

}

/*
INSERT FUNCTIONS USED FOR TESTING HERE

static void test_memory_initialisation(int64_t* memStart)
{
  int64_t* memptr = memStart;
  for (int i = 0; i < 100; i++)
  {
    int j = MEMORY_SIZE - i;
    // Testing initialised for first 100 elements
    printf("memory at address %li: %ld\n", i * sizeof(int64_t), *memptr++);
    // Testing initialisation for last 100 elements
    printf("memory at address %li: %ld\n", j * sizeof(int64_t), memptr[j]);
  }
  // Testing that the next index after the last element is uninitialised:
  const int64_t* nextLong = memStart + MEMORY_SIZE * sizeof(int64_t);
  printf("Testing uninitialised at address %li: %ld\n", MEMORY_SIZE + sizeof(int64_t), *nextLong);
  // Gave a segmentation fault for final test - assuming this is right but test may be incorrect
}

static void test_IDP_instructions() {
  // Instruction to 1 + 0 and put in first register
  immediate_data_processing(0b00010001000000000000010000100000);
  printf("1.\nreg1:%ld\nreg2:%ld\n", genRegisters[0], genRegisters[1]);
  // Add 5 to r0 and place in r1
  immediate_data_processing(0b00010001000000000001010000000001);
  printf("2.\nreg1:%ld\nreg2:%ld\n", genRegisters[0], genRegisters[1]);
  // Sub 1 from r1 and put in r2
  immediate_data_processing(0b01010001000000000000010000100010);
  printf("3.\nreg1:%ld\nreg2:%ld\nreg3:%ld\n", genRegisters[0], genRegisters[1], genRegisters[2]);
  // Sub 1 from r3 and put in r4 to test 32 and 64 bit limits.
  immediate_data_processing(0b01010001000000000000010001100011);
  immediate_data_processing(0b11010001000000000000010010000100);
  printf("4.\nreg3:%ld\nreg4:%ld\n", genRegisters[3], genRegisters[4]);
  // Test overflow set reg[5] to LLONG_Max and add 1
  genRegisters[5] = INT_MAX;
  immediate_data_processing(0b00110001000000000000010010100101);
  printf("5.\nreg5:%ld\n", genRegisters[5]);
  // Wide Move Tests
  // Moves the negation of 1 into genReg[6] 64 bit
  immediate_data_processing(0b10010010100000000000000000100110);
  // Moves 1 into genReg[7] 32 bit
  immediate_data_processing(0b01010010100000000000000000100111);
  // Moves Inserts 0x1111 into reg 8 which is filled with 0xfff...
  genRegisters[8] = 0xffffffffffffffff;
  immediate_data_processing(0b11110010101000100010001000101000);
  genRegisters[9] = 0x7ffffffe; // arithmeticRight(0b11111111111111111111111111111100, 0, 1);
}

static void test_dpr() {
  // add instruction for registers 0 and 1 stored in 2 - 1 + 2 (lsl by shamt 0, then add 2):
  genRegisters[0] = 1;
  genRegisters[1] = 2;
  int32_t add64One = 0b10001011000000000000000000100010;
  registerDataProcessing(add64One);
  printf("register 2: %li\n", genRegisters[2]);
  if (genRegisters[2] == 3) {
    printf("Success - add64One - dpr\n");
  }
  // lsl - shamt = 3; sub; result = 2 - 1 * 2^3= -6
  int32_t add64Two = 0b11001011000000000000110000100011;
  registerDataProcessing(add64Two);
  printf("register 3: %li\n", genRegisters[3]);
  if (genRegisters[3] == 6) {
    printf("Success - add64One - dpr\n");
  }

  // add, sub, lsl tested so far - but not with flags

  genRegisters[4] = 8;
  // lsr(r4, 3) - r0 = 0
  // flags: zero only
  int32_t subs32lsr = 0b01101011010001000000110000000101;
  registerDataProcessing(subs32lsr);
  printf("register 5: %li\n", genRegisters[5]);
  if (genRegisters[5] == 0 && psregisters.zero && !psregisters.neg && !psregisters.overflow && !psregisters.carry) {
    printf("Success - add64One - dpr\n");
  }
  initialise_registers();
}

static void test_utilities() {
  //genRegisters[0] = widthMask(0xabcdef01a3456789, 0);
  //genRegisters[1] = widthMask(0xabcdef01a3456789, 1);
  genRegisters[2] = rotateRight(0xabcdef011a456789, 0, 4);
  genRegisters[3] = rotateRight(0xabcdef011a456789, 1, 4);
  //genRegisters[4] = signedMask(0xbf0a2369, 0, 10);
  //genRegisters[5] = signedMask(0xbf0a2369, 11, 10);
  //genRegisters[6] = signedMask(0xbf0a2369, 30, 2);
}

static void test_branch() {
  psregisters.neg = 0;
  psregisters.overflow = 1;
  psregisters.zero = 1;
  decodeBranchInstruction(0b01010100111100001010010111001110);
  genRegisters[0] = pCounter;
  //genRegisters[1] = 
  //genRegisters[2] =
  //genRegisters[3] =
}

TEST FILES END
*/
