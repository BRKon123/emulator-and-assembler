#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <stdint.h>
#define NUM_REGISTERS 31
#define MEMORY_SIZE (1 << 18)

// Each register is represented by a 64-bit long (except for PSTATE regSize)
typedef int64_t regSize;

struct PSTATE {
  bool neg;
  bool zero;
  bool carry;
  bool overflow;
};

extern regSize genRegisters[NUM_REGISTERS];
extern regSize pCounter;
extern regSize stackPtr;
extern struct PSTATE psregisters;
extern const regSize ZERO_R;
extern int64_t *memory;

#endif