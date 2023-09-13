#ifndef EMULATOR_UTILS_H
#define EMULATOR_UTILS_H

#include <stdint.h>
#include <stdlib.h>

extern int32_t bitMask(int32_t bits, int start, int num);
extern int32_t signedMask(int32_t bits, int start, int num);
extern int64_t widthMask(int64_t bits, int control);
extern int64_t logicalLeft(int64_t bits, int control, int shamt);
extern int64_t logicalRight(int64_t bits, int control, int shamt);
extern int64_t arithmeticRight(int64_t bits, int control, int shamt);
extern int64_t rotateRight(int64_t bits, int control, int shamt);
extern int8_t getBit(int64_t bits, int32_t bitNum);
extern void output_processor_state(int64_t* memStart, char * argv);

#endif 