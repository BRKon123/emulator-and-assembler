
#include "hashTable.h"

#define MAX_LINES 1000
#define MAX_TOKENS_PER_LINE 8
#define MAX_TOKEN_LENGTH 30
#define DELIMITERS " \t\n,"
extern HashTable* symbolTable;
extern int instruction_processor(int argc, char** argv);
extern void process_assembly_file(const char* inputFilename, const char* outputFilename);
extern int currentAddress;
