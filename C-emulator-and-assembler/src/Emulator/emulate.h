#include "utilities.h"
#include "data.h"

#define HALT 0x8A000000
#define NOP  0xD503201F

// Test functions declared as prototypes so they appear last in this file
static void initialise_registers();
extern void immediate_data_processing();
extern void decodeBranchInstruction();
extern void registerDataProcessing(int32_t);
extern void loadFileIntoMemory();
static void runEmulator();
extern void decodeSingleDataTransferInstruction();
static void runInstruction(int32_t instruction);
extern void output_processor_state(int64_t* memStart, char* argv);