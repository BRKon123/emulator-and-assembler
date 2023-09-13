#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "utilities.h"

#define SIMM19_LENGTH 19
#define REG_LENGTH 5
#define OFFSET_LENGTH 12
#define UPPER_32_BITS_MASK 0xFFFFFFFF00000000
#define LOWER_32_BITS_MASK 0x00000000FFFFFFFF
#define MODE32_BYTES 4
#define MODE64_BYTES 8

static void load(uint32_t rtIndex, uint64_t address, uint32_t sizeFlag, int64_t *memStart);
static void store(uint32_t rtIndex, uint64_t address, uint32_t sizeFlag, int64_t *memStart);

void decodeSingleDataTransferInstruction(uint32_t instruction, int64_t *memStart) {
    //get the most signifcant bit to determine whehter load literal or sdt
  uint32_t lastBit = bitMask(instruction,31,1);
  uint32_t sizeFlag = bitMask(instruction,30,1);
  uint32_t rtIndex = bitMask(instruction,0,REG_LENGTH);

  if(lastBit) { 
     //this is a single data transfer
    uint32_t unsignedOffsetFlag = bitMask(instruction,24,1);
    uint32_t transferTypeFlag = bitMask(instruction,22,1);
    uint32_t xnIndex = bitMask(instruction,5,REG_LENGTH);
    uint64_t address;

    if(unsignedOffsetFlag) {
      //here the address is offset + rt register
      uint64_t offset = bitMask(instruction,10,OFFSET_LENGTH);
      offset = sizeFlag ? offset << 3 : offset << 2; //shift the offset depening on sign flag
      address = offset + genRegisters[xnIndex];
    } else {
      
      uint32_t rgOffsetFlag = bitMask(instruction,21,1);

      if(rgOffsetFlag) {
        //address calculated using register offset
        int32_t xmIndex = bitMask(instruction,16,REG_LENGTH);
        address = genRegisters[xnIndex] + genRegisters[xmIndex];
      } else {
        uint32_t prePostFlag = bitMask(instruction,11,1);
        int64_t simm9 = signedMask(instruction,12,9);
        if(prePostFlag) {
          //we are in pre indexing mode
          address = genRegisters[xnIndex] + simm9;
        } else {
          //we are in post indexing mode
          address = genRegisters[xnIndex];
        }
        genRegisters[xnIndex] = genRegisters[xnIndex] + simm9;
      }
            
    }

    if(transferTypeFlag) {
        load(rtIndex,address,sizeFlag,memStart);
    } else {
      printf("%lx\n", address);
        store(rtIndex,address,sizeFlag,memStart);
    }

  } else {
    //this is a load literal. add pc to simm19
    int32_t offset_32 = instruction << 8;
    int64_t offset_64 = offset_32 >> 13;
    // printf("addy:%ld\n", offset_64);
    int64_t address = pCounter +  ( offset_64 << 2); //add pc to this
    load(rtIndex,address,sizeFlag,memStart);
  }
}

//functions to load and store, memory is byte addressable so address can access any of the 2^21 bytes.//
//get specified byte from input data, byte index 0 is the most significant 8 bits of the input dat
static uint32_t getByteFromLong(int64_t data, uint32_t byteIndex) {
  int64_t bitMask = 0xFF; //bit mask to get the least significant 8 bits
  uint32_t numShiftBits = (byteIndex * sizeof(int64_t)); //the number of bits to shift by
  // printf("%lx\n",data);
  return (data >> numShiftBits) & bitMask;

}

//load a byte into data the input parameter points to at the specificed byte index. 
//eg. byte index of 7 would load into the 8 most significant bits of data
static void loadByteIntoLong(int64_t *data, uint64_t byte, uint32_t byteIndex) {
  int64_t mask = (int64_t)byte << (byteIndex * 8);
  *data = (*data & ~((int64_t)0xFF << (byteIndex * 8))) | mask;
}

void printBits(int64_t value) {
    size_t numBits = sizeof(int64_t) * 8;

    for (int i = numBits - 1; i >= 0; i--) {
        int bit = (value >> i) & 1;
        printf("%d", bit);
    }
    printf("\n");
}

static uint32_t getByteFromRegisters(uint32_t rtIndex, uint32_t byteIndex) {
  return getByteFromLong(genRegisters[rtIndex], byteIndex);
}

uint32_t getByteFromMemory(uint64_t address, int64_t *memStart) {
    
  memory = memStart;
  
  uint32_t memoryArrayIndex = address / sizeof(int64_t);
  uint32_t byteIndex = address % sizeof(int64_t);
  // printf("byteindex: %d\nmemadd: %d\n", byteIndex, memoryArrayIndex);
  return getByteFromLong(memory[memoryArrayIndex],byteIndex);
}

static void loadByteIntoMemory(uint64_t address, uint64_t byte, int64_t *memStart) {
    memory = memStart;
    
    uint32_t memoryArrayIndex = address / sizeof(int64_t);
    uint32_t byteIndex = address % sizeof(int64_t);
    printf("the memory index is %d, byte index %d \n",memoryArrayIndex,byteIndex);
    loadByteIntoLong(&memory[memoryArrayIndex],byte, byteIndex);
}

static void store(uint32_t rtIndex, uint64_t address, uint32_t sizeFlag, int64_t *memStart) {
  
  int8_t numBytes = sizeFlag ? MODE64_BYTES : MODE32_BYTES; //number of bytes in load - depends on size flag

  for(uint64_t i = 0; i<numBytes; i++) {
    uint32_t currentByte = getByteFromRegisters(rtIndex, i); //get the specified byte from memory
    //loadByteIntoLong(&memory[address],currentByte,correctedByteIndex);//load byte into specific location of memeory
    loadByteIntoMemory(address+i,currentByte,memStart);
  }
  
}

static void load(uint32_t rtIndex, uint64_t address, uint32_t sizeFlag, int64_t *memStart) {

  memory = memStart;

  int8_t numBytes = sizeFlag ? MODE64_BYTES: MODE32_BYTES; //number of bytes in load - depends on size flag

  for(uint64_t i = 0; i<numBytes; i++) {
    uint32_t currentByte = getByteFromMemory(address + i,memStart); //get the specified byte from memory
    loadByteIntoLong(&genRegisters[rtIndex],currentByte,i); //load byte into specific location of memeory
  }
  
}
