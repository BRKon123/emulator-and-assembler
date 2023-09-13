#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"



//IMPORTANT FOR CODE REDUNDANCY:
// For the above, can't we just "return (int32_t) bitMask64(bits, start, num);"

int64_t bitMask64(int64_t bits, int start, int num) {
  int64_t mask = pow(2, num) - 1;
  int64_t shiftedBits = bits >> start; 
  return(mask & shiftedBits);
}

/*
bitMask function to extract specified bits from a given 32 bit number
@param 'bits' represents the given 32 bit number
@param 'start' specifies the start index of the bits to be extracted
@param 'num' specifies the number of bits to be extracted beginning from start index
*/
int32_t bitMask(int32_t bits, int start, int num) {
  return (int32_t)bitMask64(bits,start,num);
}

/*
getBit function returns either a 1 or a 0 representing a 64-bit from an integer represented in binary
@param 'bits' takes the 64-bit value stored in memory
@param 'bitIndex' represents the bit number desired from 'bits'. Input range 0 to 63.
*/
int8_t getBit(int64_t bits, int32_t bitNum) {
  return (bits >> bitNum) & 0x1;
}

/*
signedMask function is used to extract signed data values from a given 32 bit number
@param 'bits' represents the given 32 bit number
@param 'start' specifies the start index of the bits to be extracted
@param 'num' specifies the number of bits to be extracted beginning from start index
*/
int32_t signedMask(int32_t bits, int start, int num) {
  int32_t unsignedBits = bitMask(bits, start, num);
  if (bitMask(unsignedBits, num - 1, 1)) {
    int32_t signedBits = -1;
    signedBits = signedBits - (pow(2, num) - 1) + unsignedBits;
    return signedBits;
  }
  else {
    return unsignedBits;
  }
}


/*
instructionMask function to extract 32 bit instructions from 64 bit memory
@param 'bits' takes the 64 bit value stored in the memory
@param 'control' can be either 0 or 1. If control is 0, we extract the rightmost 32 bits
If control is 1, we extract the leftmost 32 bits.
This sign extends the leftmost 32 bits if stored in int64_t, which can be ignored since 
we will only use the rightmost 32 bits returned by a function.
*/
int64_t widthMask(int64_t bits, int control) {
  int64_t mask = pow(2, 32) - 1;
  int64_t shiftedBits = (control) ? bits >> 32 : bits;
  return (shiftedBits & mask);
}

/*
Control 0 - we perform shifting on lower 32 bits
Control 1 - we perform shifting on entire 64 bits
*/

//performs logical left shift
int64_t logicalLeft(int64_t bits, int control, int shamt) {
  int64_t shiftedBits = 0;
  if (control == 0) {
    shiftedBits = widthMask(bits, 0) << shamt;
    int64_t mask = pow(2, 32) - 1;
    shiftedBits = shiftedBits & mask;
  } else {
    shiftedBits = bits << shamt;
  }
  return shiftedBits;
}

//performs logical right shift
//given shifting a signed integer will carry the MSB while shifting
int64_t logicalRight(int64_t bits, int control, int shamt) {
  int64_t shiftedBits = 0;
  if (control == 0) {
    shiftedBits = widthMask(bits, 0) >> shamt;
    int64_t mask = pow(2, 32) - 1;
    shiftedBits = shiftedBits & mask;
  } else {
    shiftedBits = bits;
    if (bitMask64(shiftedBits, 63, 1) == 1) {
      shiftedBits = shiftedBits >> shamt;
      int64_t sub = pow(2, shamt) - 1;
      sub = sub << (64 - shamt);
      shiftedBits = shiftedBits - sub;
    } else {
      shiftedBits = shiftedBits >> shamt;
    }
  }
  return shiftedBits;
}

//performs arithmetic right shift
int64_t arithmeticRight(int64_t bits, int control, int shamt) {
  int64_t shiftedBits = 0;
  if (control == 0) {
    shiftedBits = widthMask(bits, 0);
    if (bitMask(shiftedBits, 31, 1) == 1) {
      int64_t sub = pow(2, shamt) - 1;
      sub = sub << (32 - shamt);
      shiftedBits = shiftedBits >> shamt;
      return (shiftedBits + sub);
    }
    else {
      return (shiftedBits >> shamt);
    }
  }
  else {
    shiftedBits = bits;
    return (shiftedBits >> shamt);
  }
}

//performs rotate right
int64_t rotateRight(int64_t bits, int control, int shamt) {
  int64_t rotateBits = bitMask(bits, 0, shamt);
  int64_t shiftedBits = 0;
  if (control == 0) {
    shiftedBits = widthMask(bits, 0);
    rotateBits = rotateBits << (32 - shamt);
    shiftedBits = (shiftedBits >> shamt) + rotateBits;
    return shiftedBits;
  }
  else {
    return ((uint64_t)bits >> shamt) | ((uint64_t)bits << (64 - shamt));
  }
}

/*
output_processor_state
Note memory has not been properly tested.
When using this to debug memory note this may not be accurate.
I have only done limmited testing for memory.
*/
void output_processor_state(int64_t* memStart, char* name) {
  // consider changing the file name depending on input?
  const char* filename = strcat(strtok(name, ".") == NULL ? name : strtok(name, "."), ".out"); // get pointer to filename
  memory = memStart;
  FILE* file = fopen(filename, "w");
  if (file == NULL)
  {
    printf("Failed to open the file.\n");
    return;
  }
  // Loop through registers and output formated
  fprintf(file, "%s", "Registers:\n");
  for (int i = 0; i < NUM_REGISTERS; i++) {
    fprintf(file, "X%02d    = %016lx\n", i, genRegisters[i]);
  }
  // The rest of this is fairly intuitive.
  fprintf(file, "PC     = %016lx\n", pCounter);
  fprintf(file, "PSTATE : %s%s%s%s\n", ((psregisters.neg) ? "N" : "-"), ((psregisters.zero) ? "Z" : "-"), ((psregisters.carry) ? "C" : "-"), ((psregisters.overflow) ? "V" : "-"));
  fprintf(file, "%s", "Non-Zero Memory:\n");

  // Memory outputting...
  for (int i = 0; i < MEMORY_SIZE; i++) {
    if (widthMask(memory[i], 0) != 0) {
      fprintf(file, "0x%08x : %08x\n", i * 8,(int32_t) widthMask(memory[i], 0));
    }
    if (widthMask(memory[i], 1) != 0) {
      fprintf(file, "0x%08x : %08x\n", (i * 8) + 4,(int32_t) widthMask(memory[i], 1));
    }
  }
  // Trivial.
  fclose(file);
}


